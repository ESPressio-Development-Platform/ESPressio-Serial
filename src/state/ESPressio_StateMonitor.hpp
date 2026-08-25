#pragma once

#if !__has_include(<ESPressio_State.hpp>)
#error "StateMonitor requires ESPressio State. Include the State working branch in the consuming project when using this optional monitor."
#endif

#include <Arduino.h>
#include <ESPressio_State.hpp>

namespace ESPressio::Serial {

class StateMonitor final :
    public ESPressio::State::IRemoteStateManagerObserver,
    public ESPressio::State::IStateSubscriptionRegistryObserver,
    public ESPressio::State::IStatePublicationObserver {
private:
    Print* _output = nullptr;
    ESPressio::Observable::ObserverHandlePtr _remoteStateHandle;
    ESPressio::Observable::ObserverHandlePtr _subscriptionHandle;
    ESPressio::Observable::ObserverHandlePtr _publicationHandle;

    static const char* AvailabilityName(
        ESPressio::State::RemoteDeviceAvailability value
    ) {
        using Availability = ESPressio::State::RemoteDeviceAvailability;
        switch (value) {
            case Availability::Unknown: return "Unknown";
            case Availability::Connected: return "Connected";
            case Availability::Stale: return "Stale";
            case Availability::Disconnected: return "Disconnected";
            case Availability::ConnectionLost: return "ConnectionLost";
        }
        return "Unknown";
    }

    void Device(const ESPressio::State::DeviceIdentifier& identifier) {
        if (!_output) return;
        const auto& bytes = identifier.Bytes();
        for (std::size_t index = 0; index < bytes.size(); ++index) {
            if (index) _output->print(':');
            if (bytes[index] < 0x10) _output->print('0');
            _output->print(bytes[index], HEX);
        }
    }

    void Prefix(const char* operation) {
        if (!_output) return;
        _output->print("[ESPressio State] ");
        _output->print(operation);
    }

    void Type(ESPressio::State::StateTypeId typeId) {
        if (!_output) return;
        _output->print(" type=0x");
        _output->print(static_cast<unsigned long long>(typeId), HEX);
    }

public:
    template<typename TContract, std::size_t TMaximumDevices>
    bool ObserveRemoteState(
        ESPressio::State::RemoteStateManager<TContract, TMaximumDevices>& manager,
        Print& output
    ) {
        if (_remoteStateHandle) return true;
        _output = &output;
        _remoteStateHandle = manager.RegisterObserver(this);
        if (!_remoteStateHandle) {
            _output = nullptr;
            return false;
        }
        return true;
    }

    template<std::size_t TCapacity>
    bool ObserveSubscriptions(
        ESPressio::State::StateSubscriptionRegistry<TCapacity>& registry,
        Print& output
    ) {
        if (_subscriptionHandle) return true;
        _output = &output;
        _subscriptionHandle = registry.RegisterObserver(this);
        if (!_subscriptionHandle) {
            if (!_remoteStateHandle && !_publicationHandle) _output = nullptr;
            return false;
        }
        return true;
    }

    template<typename TPublicationObservable>
    bool ObservePublications(
        TPublicationObservable& observable,
        Print& output
    ) {
        if (_publicationHandle) return true;
        _output = &output;
        _publicationHandle = observable.RegisterObserver(this);
        if (!_publicationHandle) {
            if (!_remoteStateHandle && !_subscriptionHandle) _output = nullptr;
            return false;
        }
        return true;
    }

    void Shutdown() {
        _publicationHandle.reset();
        _subscriptionHandle.reset();
        _remoteStateHandle.reset();
        _output = nullptr;
    }

    void OnRemoteStateDeviceRegistered(
        const ESPressio::State::DeviceIdentifier& identifier
    ) override {
        Prefix("DeviceRegistered");
        _output->print(" device=");
        Device(identifier);
        _output->println();
    }

    void OnRemoteStateAccepted(
        const ESPressio::State::DeviceIdentifier& identifier,
        ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateEpoch epoch,
        ESPressio::State::StateRevision revision,
        bool changed
    ) override {
        Prefix("Accepted");
        _output->print(" device=");
        Device(identifier);
        Type(typeId);
        _output->print(" epoch=");
        _output->print(epoch);
        _output->print(" revision=");
        _output->print(static_cast<unsigned long long>(revision));
        _output->print(" changed=");
        _output->println(changed ? "true" : "false");
    }

    void OnRemoteStateRejected(
        const ESPressio::State::DeviceIdentifier& identifier,
        ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateEpoch epoch,
        ESPressio::State::StateRevision revision
    ) override {
        Prefix("Rejected");
        _output->print(" device=");
        Device(identifier);
        Type(typeId);
        _output->print(" epoch=");
        _output->print(epoch);
        _output->print(" revision=");
        _output->println(static_cast<unsigned long long>(revision));
    }

    void OnRemoteStateAvailabilityChanged(
        const ESPressio::State::DeviceIdentifier& identifier,
        ESPressio::State::RemoteDeviceAvailability previous,
        ESPressio::State::RemoteDeviceAvailability current
    ) override {
        Prefix("Availability");
        _output->print(" device=");
        Device(identifier);
        _output->print(" previous=");
        _output->print(AvailabilityName(previous));
        _output->print(" current=");
        _output->println(AvailabilityName(current));
    }

    void OnStateSubscribed(
        ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateSubscriptionScope,
        const ESPressio::State::DeviceIdentifier& device
    ) override {
        Prefix("Subscribed");
        Type(typeId);
        if (!device.IsZero()) {
            _output->print(" device=");
            Device(device);
        } else {
            _output->print(" device=ANY");
        }
        _output->println();
    }

    void OnStateUnsubscribed(
        ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateSubscriptionScope,
        const ESPressio::State::DeviceIdentifier& device
    ) override {
        Prefix("Unsubscribed");
        Type(typeId);
        if (!device.IsZero()) {
            _output->print(" device=");
            Device(device);
        } else {
            _output->print(" device=ANY");
        }
        _output->println();
    }

    void OnStateSubscriptionCapacityExhausted(
        ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateSubscriptionScope,
        const ESPressio::State::DeviceIdentifier& device
    ) override {
        Prefix("SubscriptionCapacityExhausted");
        Type(typeId);
        if (!device.IsZero()) {
            _output->print(" device=");
            Device(device);
        }
        _output->println();
    }

    void OnStatePublicationPending(
        const ESPressio::State::DeviceIdentifier& device,
        ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateEpoch epoch,
        ESPressio::State::StateRevision revision
    ) override {
        Prefix("PublicationPending");
        _output->print(" device=");
        Device(device);
        Type(typeId);
        _output->print(" epoch=");
        _output->print(epoch);
        _output->print(" revision=");
        _output->println(static_cast<unsigned long long>(revision));
    }

    void OnStatePublicationSuperseded(
        const ESPressio::State::DeviceIdentifier& device,
        ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateEpoch previousEpoch,
        ESPressio::State::StateRevision previousRevision,
        ESPressio::State::StateEpoch epoch,
        ESPressio::State::StateRevision revision
    ) override {
        Prefix("PublicationSuperseded");
        _output->print(" device=");
        Device(device);
        Type(typeId);
        _output->print(" previous=");
        _output->print(previousEpoch);
        _output->print(':');
        _output->print(static_cast<unsigned long long>(previousRevision));
        _output->print(" latest=");
        _output->print(epoch);
        _output->print(':');
        _output->println(static_cast<unsigned long long>(revision));
    }

    void OnStatePublicationAcknowledged(
        const ESPressio::State::DeviceIdentifier& device,
        ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateEpoch epoch,
        ESPressio::State::StateRevision revision
    ) override {
        Prefix("Acknowledged");
        _output->print(" device=");
        Device(device);
        Type(typeId);
        _output->print(" epoch=");
        _output->print(epoch);
        _output->print(" revision=");
        _output->println(static_cast<unsigned long long>(revision));
    }

    void OnStatePublicationStaleAcknowledgement(
        const ESPressio::State::DeviceIdentifier& device,
        ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateEpoch epoch,
        ESPressio::State::StateRevision revision
    ) override {
        Prefix("StaleAcknowledgement");
        _output->print(" device=");
        Device(device);
        Type(typeId);
        _output->print(" epoch=");
        _output->print(epoch);
        _output->print(" revision=");
        _output->println(static_cast<unsigned long long>(revision));
    }
};

} // namespace ESPressio::Serial
