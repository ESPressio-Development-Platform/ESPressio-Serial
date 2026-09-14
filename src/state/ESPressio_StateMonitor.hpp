#pragma once

#if !__has_include(<ESPressio_StateDescriptor.hpp>) || !__has_include(<ESPressio_TypeDirectory.hpp>)
#error "StateMonitor requires final ESPressio State descriptor and Primitive TypeDirectory surfaces."
#endif

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include <ESPressio_StateDescriptor.hpp>
#include <ESPressio_TypeDirectory.hpp>

#include "../console/ESPressio_Console.hpp"

namespace ESPressio::Serial {

enum class StateMonitorAuthorizationDecision : std::uint8_t {
    Authorized,
    Denied
};

/// Application-owned authorization for operator State inspection.
class IStateMonitorAuthorizer {
public:
    virtual ~IStateMonitorAuthorizer() = default;
    virtual StateMonitorAuthorizationDecision Authorize(
        const Primitive::PrimitiveTypeDescriptor& descriptor
    ) const noexcept = 0;
};

/// Bounded read-only State diagnostics over the final family descriptor surface.
/// This monitor never acquires State ownership, mutates authoritative State or owns convergence/session/transport behavior.
template<std::size_t TMaximumPayloadBytes = 2048>
class StateMonitor final {
    static_assert(TMaximumPayloadBytes > 0, "StateMonitor requires nonzero bounded output capacity");

    Primitive::TypeDirectoryView _types{};
    Print* _output = nullptr;
    const IStateMonitorAuthorizer* _authorizer = nullptr;
    std::array<std::uint8_t, TMaximumPayloadBytes> _payload{};

    static const char* TierName(State::StateTier tier) noexcept {
        switch (tier) {
            case State::StateTier::Local: return "Local";
            case State::StateTier::Serializable: return "Serializable";
            case State::StateTier::Transmissible: return "Transmissible";
        }
        return "Unknown";
    }

    static const char* ReliabilityName(Timing::TimeReliability reliability) noexcept {
        switch (reliability) {
            case Timing::TimeReliability::Unknown: return "Unknown";
            case Timing::TimeReliability::SoftwareUnbounded: return "SoftwareUnbounded";
            case Timing::TimeReliability::Holdover: return "Holdover";
            case Timing::TimeReliability::Synchronized: return "Synchronized";
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
    bool Initialize(
        Primitive::TypeDirectoryView types,
        Print& output,
        const IStateMonitorAuthorizer& authorizer
    ) noexcept {
        if (!types.IsFrozen()) return false;
        _types = types;
        _output = &output;
        _authorizer = &authorizer;
        return true;
    }

    void Shutdown() noexcept {
        _types = {};
        _output = nullptr;
        _authorizer = nullptr;
    }

    bool GetIsInitialized() const noexcept {
        return _output != nullptr && _authorizer != nullptr && _types.IsFrozen();
    }

    void List() const noexcept {
        if (!GetIsInitialized()) return;
        _output->println("Registered State Types:");
        std::size_t count = 0;
        for (const auto& common : _types) {
            if (common.Key.Family != State::StateFamilyId) continue;
            const auto* descriptor = State::GetStateTypeDescriptor(common);
            if (!descriptor) continue;
            ++count;
            _output->print("  ");
            _output->print(common.CanonicalName);
            _output->print(" type=0x");
            PrintTypeId(common.Key.TypeValue);
            _output->print(" tier=");
            _output->print(TierName(descriptor->Tier));
            _output->print(" value=");
            _output->println(descriptor->HasValue && descriptor->HasValue() ? "present" : "absent");
        }
        if (count == 0) _output->println("  <none>");
    }

    bool Describe(std::string_view canonicalName) const noexcept {
        if (!GetIsInitialized() || canonicalName.empty()) return false;
        const auto* common = _types.Find(State::StateFamilyId, canonicalName);
        if (!common) return false;
        const auto* descriptor = State::GetStateTypeDescriptor(*common);
        if (!descriptor) return false;

        _output->print("State: "); _output->println(common->CanonicalName);
        _output->print("Type ID: 0x"); PrintTypeId(common->Key.TypeValue); _output->println();
        _output->print("Tier: "); _output->println(TierName(descriptor->Tier));
        _output->print("Value bytes: "); _output->println(descriptor->ValueBytes);
        _output->print("Runtime bytes: "); _output->println(descriptor->RuntimeBytes);
        _output->print("Current value: ");
        _output->println(descriptor->HasValue && descriptor->HasValue() ? "present" : "absent");
        _output->print("JSON value bytes: ");
        _output->println(descriptor->ValueSchema ? descriptor->ValueSchema->MaximumJsonBytes : 0);
        return true;
    }

    State::StateDynamicReadStatus ReadJson(std::string_view canonicalName) noexcept {
        if (!GetIsInitialized() || canonicalName.empty()) return State::StateDynamicReadStatus::UnsupportedFormat;
        const auto* common = _types.Find(State::StateFamilyId, canonicalName);
        if (!common) {
            _output->println("State type is not registered.");
            return State::StateDynamicReadStatus::UnsupportedFormat;
        }
        const auto* descriptor = State::GetStateTypeDescriptor(*common);
        if (!descriptor || !descriptor->ValueSchema) {
            _output->println("State type is not serializable for generic inspection.");
            return State::StateDynamicReadStatus::UnsupportedFormat;
        }
        if (_authorizer->Authorize(*common) != StateMonitorAuthorizationDecision::Authorized) {
            _output->println("State inspection is not authorized.");
            return State::StateDynamicReadStatus::UnsupportedFormat;
        }

        const auto maximum = descriptor->ValueSchema->MaximumJsonBytes;
        if (maximum == 0 || maximum > _payload.size()) {
            _output->println("State JSON representation exceeds monitor capacity.");
            return State::StateDynamicReadStatus::InsufficientOutput;
        }

        const auto result = State::ReadDynamicState(
            *descriptor,
            State::StatePayloadFormat::JSON,
            _payload.data(),
            _payload.size());

        switch (result.Status) {
            case State::StateDynamicReadStatus::Success:
                _output->print("State value: ");
                _output->print(std::string_view(
                    reinterpret_cast<const char*>(_payload.data()), result.Bytes));
                _output->println();
                _output->print("Truth time ns: "); _output->println(result.TruthTime.Nanoseconds);
                _output->print("Truth reliability: "); _output->println(ReliabilityName(result.TruthTime.Reliability));
                break;
            case State::StateDynamicReadStatus::NoValue:
                _output->println("State has no current value.");
                break;
            case State::StateDynamicReadStatus::InsufficientOutput:
                _output->println("State representation exceeds monitor capacity.");
                break;
            case State::StateDynamicReadStatus::SerializationFailure:
                _output->println("State serialization failed.");
                break;
            case State::StateDynamicReadStatus::UnsupportedFormat:
                _output->println("State JSON representation is unavailable.");
                break;
        }
        return result.Status;
    }
};

} // namespace ESPressio::Serial
