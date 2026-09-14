#pragma once

#if !__has_include(<ESPressio_EventTypeDescriptor.hpp>) || !__has_include(<ESPressio_TypeDirectory.hpp>)
#error "EventConsole requires final ESPressio Event descriptor and Primitive TypeDirectory surfaces."
#endif

#include <cstddef>
#include <cstdint>
#include <string_view>

#include <ESPressio_EventTypeDescriptor.hpp>
#include <ESPressio_TypeDirectory.hpp>

#include "../console/ESPressio_Console.hpp"
#include "ESPressio_EventConsoleTypes.hpp"

namespace ESPressio::Serial {

enum class EventConsoleAuthorizationDecision : std::uint8_t {
    Authorized,
    Denied
};

/// Application-owned authorization for operator-origin Event dispatch.
class IEventConsoleAuthorizer {
public:
    virtual ~IEventConsoleAuthorizer() = default;
    virtual EventConsoleAuthorizationDecision Authorize(
        const Primitive::PrimitiveTypeDescriptor& descriptor
    ) const noexcept = 0;
};

/// Descriptor-driven operator Event console.
/// The console owns no Event registry, EventTransportManager, listener topology, occurrence pool, retry path or transport.
class EventConsole final {
    Console* _console = nullptr;
    Print* _output = nullptr;
    Primitive::TypeDirectoryView _types{};
    const IEventConsoleAuthorizer* _authorizer = nullptr;
    EventConsoleConfig _config{};
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

    static const char* StatusName(Event::EventDynamicDispatchStatus status) noexcept {
        switch (status) {
            case Event::EventDynamicDispatchStatus::Accepted: return "Accepted";
            case Event::EventDynamicDispatchStatus::NotConstructible: return "NotConstructible";
            case Event::EventDynamicDispatchStatus::UnsupportedFormat: return "UnsupportedFormat";
            case Event::EventDynamicDispatchStatus::SchemaOrDecodeFailure: return "SchemaOrDecodeFailure";
            case Event::EventDynamicDispatchStatus::NotInitialized: return "NotInitialized";
            case Event::EventDynamicDispatchStatus::Stopping: return "Stopping";
            case Event::EventDynamicDispatchStatus::CapacityUnavailable: return "CapacityUnavailable";
            case Event::EventDynamicDispatchStatus::IdentityUnavailable: return "IdentityUnavailable";
            case Event::EventDynamicDispatchStatus::IdentifierExhausted: return "IdentifierExhausted";
        }
        return "Unknown";
    }

    void List() const noexcept {
        if (!_output) return;
        _output->println("Registered Event Types:");
        std::size_t count = 0;
        for (const auto& common : _types) {
            if (common.Key.Family != Event::EventFamilyId) continue;
            const auto* descriptor = Event::GetEventTypeDescriptor(common);
            if (!descriptor) continue;
            ++count;
            _output->print("  "); _output->print(common.CanonicalName);
            _output->println(descriptor->DynamicallyConstructible ? " [constructible]" : " [not-constructible]");
        }
        if (count == 0) _output->println("  <none>");
    }

    void Describe(std::string_view name) const noexcept {
        if (!_output) return;
        const auto* common = _types.Find(Event::EventFamilyId, Trim(name));
        if (!common) { _output->println("Event type is not registered."); return; }
        const auto* descriptor = Event::GetEventTypeDescriptor(*common);
        if (!descriptor) { _output->println("Event descriptor is unavailable."); return; }
        _output->print("Event: "); _output->println(common->CanonicalName);
        _output->print("JSON payload bytes: ");
        _output->println(descriptor->Schema ? descriptor->Schema->MaximumJsonBytes : 0);
        _output->print("Dynamic construction: ");
        _output->println(descriptor->DynamicallyConstructible && descriptor->DispatchSerialized ? "available" : "unavailable");
        _output->print("Max live instances: "); _output->println(descriptor->MaximumLiveInstances);
        _output->print("Max pending instances: "); _output->println(descriptor->MaximumPendingInstances);
    }

    void DispatchJson(std::string_view arguments) noexcept {
        if (!_output) return;
        const auto [name, payload] = SplitFirst(arguments);
        if (name.empty() || payload.empty()) {
            _output->println("Usage: event json <canonical-type-name> <json>");
            return;
        }
        const auto* common = _types.Find(Event::EventFamilyId, name);
        if (!common) { _output->println("Event type is not registered."); return; }
        const auto* descriptor = Event::GetEventTypeDescriptor(*common);
        if (!descriptor || !descriptor->Schema) {
            _output->println("Event does not expose bounded P3 metadata.");
            return;
        }
        if (_authorizer->Authorize(*common) != EventConsoleAuthorizationDecision::Authorized) {
            _output->println("Event dispatch is not authorized.");
            return;
        }
        if (!descriptor->DynamicallyConstructible || descriptor->DispatchSerialized == nullptr) {
            _output->println("Event is not dynamically constructible.");
            return;
        }
        const auto schemaMaximum = descriptor->Schema->MaximumJsonBytes;
        if (schemaMaximum == 0 || payload.size() > schemaMaximum || payload.size() > _config.MaximumJsonLength) {
            _output->println("Event JSON exceeds bounded console/schema capacity.");
            return;
        }
        const auto result = Event::DispatchDynamicEvent(
            *descriptor,
            Event::EventPayloadFormat::JSON,
            reinterpret_cast<const std::uint8_t*>(payload.data()),
            payload.size());
        _output->print("Event dispatch: ");
        _output->println(StatusName(result.Status));
    }

    void HandleCommand(const ConsoleCommandContext& context) noexcept {
        const auto [operation, remainder] = SplitFirst(context.Arguments);
        if (operation == "describe") { Describe(remainder); return; }
        if (operation == "json") { DispatchJson(remainder); return; }
        _output->println("Usage: event describe <canonical-type-name>");
        _output->println("       event json <canonical-type-name> <json>");
    }

public:
    EventConsole() = default;
    EventConsole(const EventConsole&) = delete;
    EventConsole& operator=(const EventConsole&) = delete;
    ~EventConsole() { Shutdown(); }

    bool Initialize(
        Console& console,
        Primitive::TypeDirectoryView types,
        const IEventConsoleAuthorizer& authorizer,
        EventConsoleConfig config = {}
    ) {
        Shutdown();
        if (!console.GetIsInitialized() || console.GetOutput() == nullptr || !types.IsFrozen() || config.MaximumJsonLength == 0) return false;
        _console = &console;
        _output = console.GetOutput();
        _types = types;
        _authorizer = &authorizer;
        _config = config;
        if (!console.RegisterCommand("events", "list registered Event Types", [this](const auto&) { List(); })) {
            Shutdown(); return false;
        }
        if (!console.RegisterCommand("event", "describe or dispatch a typed Event", [this](const auto& context) { HandleCommand(context); })) {
            console.UnregisterCommand("events");
            Shutdown(); return false;
        }
        _commandsRegistered = true;
        return true;
    }

    void Shutdown() noexcept {
        if (_console && _commandsRegistered) {
            _console->UnregisterCommand("event");
            _console->UnregisterCommand("events");
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
