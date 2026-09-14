#pragma once

#if !__has_include(<ESPressio_CommandDescriptor.hpp>) || !__has_include(<ESPressio_TypeDirectory.hpp>)
#error "CommandMonitor requires final ESPressio Command descriptor and Primitive TypeDirectory surfaces."
#endif

#include <cstdint>
#include <cstdio>
#include <string_view>

#include <ESPressio_CommandDescriptor.hpp>
#include <ESPressio_TypeDirectory.hpp>

#include "../console/ESPressio_Console.hpp"

namespace ESPressio::Serial {

/// Read-only Command descriptor diagnostics for operator tooling.
/// No registry observer, execution lifecycle, queue, worker or transport is owned here.
class CommandMonitor final {
    Primitive::TypeDirectoryView _types{};
    Print* _output = nullptr;

    static const char* TierName(Command::CommandTier tier) noexcept {
        switch (tier) {
            case Command::CommandTier::Local: return "Local";
            case Command::CommandTier::Serializable: return "Serializable";
            case Command::CommandTier::Transmissible: return "Transmissible";
        }
        return "Unknown";
    }

    static const char* ConstructionName(Command::CommandDynamicConstructionMode mode) noexcept {
        switch (mode) {
            case Command::CommandDynamicConstructionMode::Unavailable: return "Unavailable";
            case Command::CommandDynamicConstructionMode::FireAndForget: return "FireAndForget";
            case Command::CommandDynamicConstructionMode::RequesterRequired: return "RequesterRequired";
        }
        return "Unknown";
    }

    void PrintTypeId(std::uint64_t value) const noexcept {
        if (!_output) return;
        char buffer[24]{};
        std::snprintf(buffer, sizeof(buffer), "%016llX", static_cast<unsigned long long>(value));
        _output->print(buffer);
    }

public:
    bool Initialize(Primitive::TypeDirectoryView types, Print& output) noexcept {
        if (!types.IsFrozen()) return false;
        _types = types;
        _output = &output;
        return true;
    }

    void Shutdown() noexcept {
        _types = {};
        _output = nullptr;
    }

    bool GetIsInitialized() const noexcept { return _output != nullptr && _types.IsFrozen(); }

    void List() const noexcept {
        if (!GetIsInitialized()) return;
        _output->println("Registered Command Types:");
        std::size_t count = 0;
        for (const auto& common : _types) {
            if (common.Key.Family != Command::CommandFamilyId) continue;
            const auto* descriptor = Command::GetCommandTypeDescriptor(common);
            if (!descriptor) continue;
            ++count;
            _output->print("  ");
            _output->print(common.CanonicalName);
            _output->print(" type=0x");
            PrintTypeId(common.Key.TypeValue);
            _output->print(" tier=");
            _output->print(TierName(descriptor->Tier));
            _output->print(" construction=");
            _output->print(ConstructionName(descriptor->DynamicConstruction));
            _output->print(" handler=");
            _output->println(descriptor->HasHandler && descriptor->HasHandler() ? "bound" : "unbound");
        }
        if (count == 0) _output->println("  <none>");
    }

    bool Describe(std::string_view canonicalName) const noexcept {
        if (!GetIsInitialized() || canonicalName.empty()) return false;
        const auto* common = _types.Find(Command::CommandFamilyId, canonicalName);
        if (!common) return false;
        const auto* descriptor = Command::GetCommandTypeDescriptor(*common);
        if (!descriptor) return false;

        _output->print("Command: "); _output->println(common->CanonicalName);
        _output->print("Type ID: 0x"); PrintTypeId(common->Key.TypeValue); _output->println();
        _output->print("Tier: "); _output->println(TierName(descriptor->Tier));
        _output->print("Dynamic construction: "); _output->println(ConstructionName(descriptor->DynamicConstruction));
        _output->print("Handler: "); _output->println(descriptor->HasHandler && descriptor->HasHandler() ? "bound" : "unbound");
        _output->print("Max live instances: "); _output->println(descriptor->MaximumLiveInstances);
        _output->print("Max pending executions: "); _output->println(descriptor->MaximumPendingExecutions);
        _output->print("Execution lanes: "); _output->println(descriptor->ExecutionLaneCount);
        _output->print("JSON request bytes: ");
        _output->println(descriptor->RequestSchema ? descriptor->RequestSchema->MaximumJsonBytes : 0);
        return true;
    }
};

} // namespace ESPressio::Serial
