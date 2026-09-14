#pragma once

#if !__has_include(<ESPressio_EventTypeDescriptor.hpp>) || !__has_include(<ESPressio_TypeDirectory.hpp>)
#error "EventMonitor requires final ESPressio Event descriptor and Primitive TypeDirectory surfaces."
#endif

#include <cstdint>
#include <cstdio>
#include <string_view>

#include <ESPressio_EventTypeDescriptor.hpp>
#include <ESPressio_TypeDirectory.hpp>

#include "../console/ESPressio_Console.hpp"

namespace ESPressio::Serial {

/// Read-only Event descriptor diagnostics for operator tooling.
/// No EventTransportManager observer, transaction queue, Task, listener topology or transport is owned here.
class EventMonitor final {
    Primitive::TypeDirectoryView _types{};
    Print* _output = nullptr;

    static const char* TierName(Event::EventTier tier) noexcept {
        switch (tier) {
            case Event::EventTier::Local: return "Local";
            case Event::EventTier::Serializable: return "Serializable";
            case Event::EventTier::Transmissible: return "Transmissible";
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
        _output->println("Registered Event Types:");
        std::size_t count = 0;
        for (const auto& common : _types) {
            if (common.Key.Family != Event::EventFamilyId) continue;
            const auto* descriptor = Event::GetEventTypeDescriptor(common);
            if (!descriptor) continue;
            ++count;
            _output->print("  ");
            _output->print(common.CanonicalName);
            _output->print(" type=0x");
            PrintTypeId(common.Key.TypeValue);
            _output->print(" tier=");
            _output->print(TierName(descriptor->Tier));
            _output->print(" constructible=");
            _output->println(descriptor->DynamicallyConstructible ? "yes" : "no");
        }
        if (count == 0) _output->println("  <none>");
    }

    bool Describe(std::string_view canonicalName) const noexcept {
        if (!GetIsInitialized() || canonicalName.empty()) return false;
        const auto* common = _types.Find(Event::EventFamilyId, canonicalName);
        if (!common) return false;
        const auto* descriptor = Event::GetEventTypeDescriptor(*common);
        if (!descriptor) return false;

        _output->print("Event: "); _output->println(common->CanonicalName);
        _output->print("Type ID: 0x"); PrintTypeId(common->Key.TypeValue); _output->println();
        _output->print("Tier: "); _output->println(TierName(descriptor->Tier));
        _output->print("Dynamic construction: ");
        _output->println(descriptor->DynamicallyConstructible && descriptor->DispatchSerialized ? "available" : "unavailable");
        _output->print("Max live instances: "); _output->println(descriptor->MaximumLiveInstances);
        _output->print("Max pending instances: "); _output->println(descriptor->MaximumPendingInstances);
        _output->print("JSON payload bytes: ");
        _output->println(descriptor->Schema ? descriptor->Schema->MaximumJsonBytes : 0);
        return true;
    }
};

} // namespace ESPressio::Serial
