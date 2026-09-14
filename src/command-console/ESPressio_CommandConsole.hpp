#pragma once

#if !__has_include(<ESPressio_CommandDescriptor.hpp>) || !__has_include(<ESPressio_TypeDirectory.hpp>)
#error "CommandConsole requires final ESPressio Command descriptor and Primitive TypeDirectory surfaces."
#endif

#include <cstddef>
#include <cstdint>
#include <string_view>

#include <ESPressio_CommandDescriptor.hpp>
#include <ESPressio_TypeDirectory.hpp>

#include "../console/ESPressio_Console.hpp"

namespace ESPressio::Serial {

enum class CommandConsoleAuthorizationDecision : std::uint8_t {
    Authorized,
    Denied
};

/// Application-owned authorization for operator-origin Command submission.
class ICommandConsoleAuthorizer {
public:
    virtual ~ICommandConsoleAuthorizer() = default;
    virtual CommandConsoleAuthorizationDecision Authorize(
        const Primitive::PrimitiveTypeDescriptor& descriptor
    ) const noexcept = 0;
};

/// Descriptor-driven operator Command console.
/// The console owns no Command registry, response route, requester, execution pool, retry path or transport.
class CommandConsole final {
    Console* _console = nullptr;
    Print* _output = nullptr;
    Primitive::TypeDirectoryView _types{};
    const ICommandConsoleAuthorizer* _authorizer = nullptr;
    bool _commandsRegistered = false;

    static std::string_view Trim(std::string_view value) noexcept {
        while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) value.remove_prefix(1);
        while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) value.remove_suffix(1);
        return value;
    }

    static std::pair<std::string_view, std::string_view> SplitFirst(std::string_view value) noexcept {
        value = Trim(value);
        const auto separator = value.find_first_of(" \t");
        if (separator == std::string_view::npos) return {value, {}};
        return {value.substr(0, separator), Trim(value.substr(separator + 1))};
    }

    static const char* StatusName(Command::CommandSubmissionStatus status) noexcept {
        switch (status) {
            case Command::CommandSubmissionStatus::Accepted: return "Accepted";
            case Command::CommandSubmissionStatus::NotInitialized: return "NotInitialized";
            case Command::CommandSubmissionStatus::Stopping: return "Stopping";
            case Command::CommandSubmissionStatus::CapacityUnavailable: return "CapacityUnavailable";
            case Command::CommandSubmissionStatus::ResponseCapacityUnavailable: return "ResponseCapacityUnavailable";
            case Command::CommandSubmissionStatus::IdentityUnavailable: return "IdentityUnavailable";
            case Command::CommandSubmissionStatus::IdentifierExhausted: return "IdentifierExhausted";
            case Command::CommandSubmissionStatus::HandlerUnavailable: return "HandlerUnavailable";
            case Command::CommandSubmissionStatus::PersistenceUnavailable: return "PersistenceUnavailable";
            case Command::CommandSubmissionStatus::LedgerCapacityUnavailable: return "LedgerCapacityUnavailable";
            case Command::CommandSubmissionStatus::SchemaOrDecodeFailure: return "SchemaOrDecodeFailure";
            case Command::CommandSubmissionStatus::InvalidRequest: return "InvalidRequest";
            case Command::CommandSubmissionStatus::InvalidTarget: return "InvalidTarget";
            case Command::CommandSubmissionStatus::TransportUnavailable: return "TransportUnavailable";
            case Command::CommandSubmissionStatus::Discarded: return "Discarded";
        }
        return "Unknown";
    }

    void List() const noexcept {
        if (!_output) return;
        _output->println("Registered Command Types:");
        std::size_t count = 0;
        for (const auto& common : _types) {
            if (common.Key.Family != Command::CommandFamilyId) continue;
            const auto* descriptor = Command::GetCommandTypeDescriptor(common);
            if (!descriptor) continue;
            ++count;
            _output->print("  "); _output->print(common.CanonicalName);
            switch (descriptor->DynamicConstruction) {
                case Command::CommandDynamicConstructionMode::FireAndForget:
                    _output->println(" [fire-and-forget]"); break;
                case Command::CommandDynamicConstructionMode::RequesterRequired:
                    _output->println(" [requester-required]"); break;
                case Command::CommandDynamicConstructionMode::Unavailable:
                    _output->println(" [not-constructible]"); break;
            }
        }
        if (count == 0) _output->println("  <none>");
    }

    void Describe(std::string_view name) const noexcept {
        if (!_output) return;
        const auto* common = _types.Find(Command::CommandFamilyId, Trim(name));
        if (!common) { _output->println("Command type is not registered."); return; }
        const auto* descriptor = Command::GetCommandTypeDescriptor(*common);
        if (!descriptor) { _output->println("Command descriptor is unavailable."); return; }
        _output->print("Command: "); _output->println(common->CanonicalName);
        _output->print("JSON request bytes: ");
        _output->println(descriptor->RequestSchema ? descriptor->RequestSchema->MaximumJsonBytes : 0);
        _output->print("Construction: ");
        switch (descriptor->DynamicConstruction) {
            case Command::CommandDynamicConstructionMode::FireAndForget: _output->println("FireAndForget"); break;
            case Command::CommandDynamicConstructionMode::RequesterRequired: _output->println("RequesterRequired"); break;
            case Command::CommandDynamicConstructionMode::Unavailable: _output->println("Unavailable"); break;
        }
    }

    void SubmitJson(std::string_view arguments) noexcept {
        if (!_output) return;
        const auto [name, payload] = SplitFirst(arguments);
        if (name.empty() || payload.empty()) {
            _output->println("Usage: command json <canonical-type-name> <json>");
            return;
        }
        const auto* common = _types.Find(Command::CommandFamilyId, name);
        if (!common) { _output->println("Command type is not registered."); return; }
        const auto* descriptor = Command::GetCommandTypeDescriptor(*common);
        if (!descriptor || !descriptor->RequestSchema) {
            _output->println("Command does not expose bounded request P3 metadata.");
            return;
        }
        if (_authorizer->Authorize(*common) != CommandConsoleAuthorizationDecision::Authorized) {
            _output->println("Command submission is not authorized.");
            return;
        }
        if (descriptor->DynamicConstruction == Command::CommandDynamicConstructionMode::RequesterRequired) {
            _output->println("Command requires requester capability; generic Serial submission is unavailable.");
            return;
        }
        if (descriptor->DynamicConstruction != Command::CommandDynamicConstructionMode::FireAndForget ||
            descriptor->SubmitSerialized == nullptr) {
            _output->println("Command is not dynamically constructible.");
            return;
        }
        const auto maximum = descriptor->RequestSchema->MaximumJsonBytes;
        if (maximum == 0 || payload.size() > maximum) {
            _output->println("Command JSON exceeds bounded schema capacity.");
            return;
        }
        const auto result = Command::SubmitDynamicCommand(
            *descriptor,
            Command::CommandPayloadFormat::JSON,
            reinterpret_cast<const std::uint8_t*>(payload.data()),
            payload.size());
        _output->print("Command submission: ");
        _output->println(StatusName(result.Status));
    }

    void HandleCommand(const ConsoleCommandContext& context) noexcept {
        const auto [operation, remainder] = SplitFirst(context.Arguments);
        if (operation == "describe") { Describe(remainder); return; }
        if (operation == "json") { SubmitJson(remainder); return; }
        _output->println("Usage: command describe <canonical-type-name>");
        _output->println("       command json <canonical-type-name> <json>");
    }

public:
    CommandConsole() = default;
    CommandConsole(const CommandConsole&) = delete;
    CommandConsole& operator=(const CommandConsole&) = delete;
    ~CommandConsole() { Shutdown(); }

    bool Initialize(
        Console& console,
        Primitive::TypeDirectoryView types,
        const ICommandConsoleAuthorizer& authorizer
    ) {
        Shutdown();
        if (!console.GetIsInitialized() || console.GetOutput() == nullptr || !types.IsFrozen()) return false;
        _console = &console;
        _output = console.GetOutput();
        _types = types;
        _authorizer = &authorizer;
        if (!console.RegisterCommand("commands", "list registered Command Types", [this](const auto&) { List(); })) {
            Shutdown(); return false;
        }
        if (!console.RegisterCommand("command", "describe or submit a typed Command", [this](const auto& context) { HandleCommand(context); })) {
            console.UnregisterCommand("commands");
            Shutdown(); return false;
        }
        _commandsRegistered = true;
        return true;
    }

    void Shutdown() noexcept {
        if (_console && _commandsRegistered) {
            _console->UnregisterCommand("command");
            _console->UnregisterCommand("commands");
        }
        _commandsRegistered = false;
        _console = nullptr;
        _output = nullptr;
        _types = {};
        _authorizer = nullptr;
    }

    bool GetIsInitialized() const noexcept {
        return _console != nullptr && _output != nullptr && _authorizer != nullptr && _types.IsFrozen() && _commandsRegistered;
    }

    Console* GetConsole() const noexcept { return _console; }
};

} // namespace ESPressio::Serial
