#pragma once

#if !__has_include(<ESPressio_State.hpp>)
#error "StateMonitor requires ESPressio State. Include the State working branch when using this optional monitor."
#endif

#include <Arduino.h>
#include <cstdio>
#include <ESPressio_State.hpp>

namespace ESPressio::Serial {

/// <summary>Writes remote-state, subscription, subscriber, publisher, and publication-tracker activity to an Arduino Print sink.</summary>
/// <remarks>Each State subsystem may be observed independently; Shutdown releases all active observer registrations.</remarks>
class StateMonitor final :
    public ESPressio::State::IRemoteStateManagerObserver,
    public ESPressio::State::IStateSubscriptionRegistryObserver,
    public ESPressio::State::IStateSubscriberRegistryObserver,
    public ESPressio::State::IStatePublisherObserver,
    public ESPressio::State::IStatePublicationObserver {
    Print* _output = nullptr;
    ESPressio::Observable::ObserverHandlePtr _remoteStateHandle;
    ESPressio::Observable::ObserverHandlePtr _subscriptionHandle;
    ESPressio::Observable::ObserverHandlePtr _subscriberHandle;
    ESPressio::Observable::ObserverHandlePtr _publisherHandle;
    ESPressio::Observable::ObserverHandlePtr _publicationHandle;

    static const char* AvailabilityName(ESPressio::State::RemoteDeviceAvailability value) {
        using A = ESPressio::State::RemoteDeviceAvailability;
        switch (value) {
            case A::Unknown: return "Unknown";
            case A::Connected: return "Connected";
            case A::Stale: return "Stale";
            case A::Disconnected: return "Disconnected";
            case A::ConnectionLost: return "ConnectionLost";
        }
        return "Unknown";
    }

    void Prefix(const char* operation) {
        if (!_output) return;
        _output->print("[ESPressio State] ");
        _output->print(operation);
    }

    void Device(const ESPressio::State::DeviceIdentifier& identifier) {
        if (!_output) return;
        const auto& bytes = identifier.Bytes();
        for (std::size_t index = 0; index < bytes.size(); ++index) {
            if (index) _output->print(':');
            char value[3];
            std::snprintf(value, sizeof(value), "%02X", static_cast<unsigned>(bytes[index]));
            _output->print(value);
        }
    }

    void Type(ESPressio::State::StateTypeId value) {
        if (!_output) return;
        char buffer[24];
        std::snprintf(buffer, sizeof(buffer), " type=0x%016llX", static_cast<unsigned long long>(value));
        _output->print(buffer);
    }

    void Revision(const char* label, ESPressio::State::StateEpoch epoch, ESPressio::State::StateRevision revision) {
        if (!_output) return;
        char buffer[48];
        std::snprintf(
            buffer,
            sizeof(buffer),
            " %s=%lu:%llu",
            label,
            static_cast<unsigned long>(epoch),
            static_cast<unsigned long long>(revision)
        );
        _output->print(buffer);
    }

public:
    /// <summary>Observes one RemoteStateManager instance.</summary>
    /// <returns>True when the remote-state observer registration is active.</returns>
    template<typename TContract, std::size_t TMaximumDevices>
    bool ObserveRemoteState(
        ESPressio::State::RemoteStateManager<TContract, TMaximumDevices>& manager,
        Print& output
    ) {
        if (_remoteStateHandle) return true;
        _output = &output;
        _remoteStateHandle = manager.RegisterObserver(
            static_cast<ESPressio::State::IRemoteStateManagerObserver*>(this)
        );
        return static_cast<bool>(_remoteStateHandle);
    }

    /// <summary>Observes one local State subscription registry.</summary>
    /// <returns>True when the subscription-registry observer registration is active.</returns>
    template<std::size_t TCapacity>
    bool ObserveSubscriptions(
        ESPressio::State::StateSubscriptionRegistry<TCapacity>& registry,
        Print& output
    ) {
        if (_subscriptionHandle) return true;
        _output = &output;
        _subscriptionHandle = registry.RegisterObserver(
            static_cast<ESPressio::State::IStateSubscriptionRegistryObserver*>(this)
        );
        return static_cast<bool>(_subscriptionHandle);
    }

    /// <summary>Observes one remote-subscriber registry.</summary>
    /// <returns>True when the subscriber-registry observer registration is active.</returns>
    template<typename TContract, std::size_t TMaximumSubscribers>
    bool ObserveSubscribers(
        ESPressio::State::StateSubscriberRegistry<TContract, TMaximumSubscribers>& registry,
        Print& output
    ) {
        if (_subscriberHandle) return true;
        _output = &output;
        _subscriberHandle = registry.RegisterObserver(
            static_cast<ESPressio::State::IStateSubscriberRegistryObserver*>(this)
        );
        return static_cast<bool>(_subscriberHandle);
    }

    /// <summary>Observes one StatePublisher instance.</summary>
    /// <returns>True when the publisher observer registration is active.</returns>
    template<typename TContract>
    bool ObservePublisher(
        ESPressio::State::StatePublisher<TContract>& publisher,
        Print& output
    ) {
        if (_publisherHandle) return true;
        _output = &output;
        _publisherHandle = publisher.RegisterObserver(
            static_cast<ESPressio::State::IStatePublisherObserver*>(this)
        );
        return static_cast<bool>(_publisherHandle);
    }

    /// <summary>Observes publication acknowledgement/supersession tracking for one State definition.</summary>
    /// <returns>True when the publication-tracker observer registration is active.</returns>
    template<typename TDefinition>
    bool ObservePublications(
        ESPressio::State::StatePublicationTracker<TDefinition>& tracker,
        Print& output
    ) {
        if (_publicationHandle) return true;
        _output = &output;
        _publicationHandle = tracker.RegisterObserver(
            static_cast<ESPressio::State::IStatePublicationObserver*>(this)
        );
        return static_cast<bool>(_publicationHandle);
    }

    /// <summary>Releases every active State observer registration and the output sink reference.</summary>
    void Shutdown() {
        _publicationHandle.reset();
        _publisherHandle.reset();
        _subscriberHandle.reset();
        _subscriptionHandle.reset();
        _remoteStateHandle.reset();
        _output = nullptr;
    }

    /// <inheritdoc/>
    void OnRemoteStateDeviceRegistered(const ESPressio::State::DeviceIdentifier& device) override {
        Prefix("DeviceRegistered"); _output->print(" device="); Device(device); _output->println();
    }

    /// <inheritdoc/>
    void OnRemoteStateAccepted(const ESPressio::State::DeviceIdentifier& device, ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateEpoch epoch, ESPressio::State::StateRevision revision, bool changed) override {
        Prefix("Accepted"); _output->print(" device="); Device(device); Type(typeId); Revision("revision", epoch, revision);
        _output->print(" changed="); _output->println(changed ? "true" : "false");
    }

    /// <inheritdoc/>
    void OnRemoteStateRejected(const ESPressio::State::DeviceIdentifier& device, ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateEpoch epoch, ESPressio::State::StateRevision revision) override {
        Prefix("Rejected"); _output->print(" device="); Device(device); Type(typeId); Revision("revision", epoch, revision); _output->println();
    }

    /// <inheritdoc/>
    void OnRemoteStateAvailabilityChanged(const ESPressio::State::DeviceIdentifier& device,
        ESPressio::State::RemoteDeviceAvailability previous, ESPressio::State::RemoteDeviceAvailability current) override {
        Prefix("Availability"); _output->print(" device="); Device(device); _output->print(" previous=");
        _output->print(AvailabilityName(previous)); _output->print(" current="); _output->println(AvailabilityName(current));
    }

    /// <inheritdoc/>
    void OnStateSubscribed(ESPressio::State::StateTypeId typeId, ESPressio::State::StateSubscriptionScope,
        const ESPressio::State::DeviceIdentifier& device) override {
        Prefix("Subscribed"); Type(typeId); _output->print(" device=");
        if (device.IsZero()) _output->print("ANY"); else Device(device); _output->println();
    }

    /// <inheritdoc/>
    void OnStateUnsubscribed(ESPressio::State::StateTypeId typeId, ESPressio::State::StateSubscriptionScope,
        const ESPressio::State::DeviceIdentifier& device) override {
        Prefix("Unsubscribed"); Type(typeId); _output->print(" device=");
        if (device.IsZero()) _output->print("ANY"); else Device(device); _output->println();
    }

    /// <inheritdoc/>
    void OnStateSubscriptionCapacityExhausted(ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateSubscriptionScope, const ESPressio::State::DeviceIdentifier& device) override {
        Prefix("SubscriptionCapacityExhausted"); Type(typeId); if (!device.IsZero()) { _output->print(" device="); Device(device); } _output->println();
    }

    /// <inheritdoc/>
    void OnRemoteStateSubscriberAdded(const ESPressio::State::DeviceIdentifier& device,
        ESPressio::State::StateTypeId typeId) override {
        Prefix("RemoteSubscriberAdded"); _output->print(" device="); Device(device); Type(typeId); _output->println();
    }

    /// <inheritdoc/>
    void OnRemoteStateSubscriberRemoved(const ESPressio::State::DeviceIdentifier& device,
        ESPressio::State::StateTypeId typeId) override {
        Prefix("RemoteSubscriberRemoved"); _output->print(" device="); Device(device); Type(typeId); _output->println();
    }

    /// <inheritdoc/>
    void OnRemoteStateSubscriberDeviceRemoved(const ESPressio::State::DeviceIdentifier& device) override {
        Prefix("RemoteSubscriberDeviceRemoved"); _output->print(" device="); Device(device); _output->println();
    }

    /// <inheritdoc/>
    void OnRemoteStateSubscriberCapacityExhausted(const ESPressio::State::DeviceIdentifier& device,
        ESPressio::State::StateTypeId typeId) override {
        Prefix("RemoteSubscriberCapacityExhausted"); _output->print(" device="); Device(device); Type(typeId); _output->println();
    }

    /// <inheritdoc/>
    void OnStateSourceRegistered(ESPressio::State::StateTypeId typeId) override {
        Prefix("SourceRegistered"); Type(typeId); _output->println();
    }

    /// <inheritdoc/>
    void OnStateSourceUnregistered(ESPressio::State::StateTypeId typeId) override {
        Prefix("SourceUnregistered"); Type(typeId); _output->println();
    }

    /// <inheritdoc/>
    void OnStatePublished(ESPressio::State::StateTypeId typeId, ESPressio::State::StateEpoch epoch,
        ESPressio::State::StateRevision revision) override {
        Prefix("Published"); Type(typeId); Revision("revision", epoch, revision); _output->println();
    }

    /// <inheritdoc/>
    void OnStatePublicationPending(const ESPressio::State::DeviceIdentifier& device, ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateEpoch epoch, ESPressio::State::StateRevision revision) override {
        Prefix("PublicationPending"); _output->print(" device="); Device(device); Type(typeId); Revision("revision", epoch, revision); _output->println();
    }

    /// <inheritdoc/>
    void OnStatePublicationSuperseded(const ESPressio::State::DeviceIdentifier& device, ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateEpoch previousEpoch, ESPressio::State::StateRevision previousRevision,
        ESPressio::State::StateEpoch epoch, ESPressio::State::StateRevision revision) override {
        Prefix("PublicationSuperseded"); _output->print(" device="); Device(device); Type(typeId);
        Revision("previous", previousEpoch, previousRevision); Revision("latest", epoch, revision); _output->println();
    }

    /// <inheritdoc/>
    void OnStatePublicationAcknowledged(const ESPressio::State::DeviceIdentifier& device, ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateEpoch epoch, ESPressio::State::StateRevision revision) override {
        Prefix("Acknowledged"); _output->print(" device="); Device(device); Type(typeId); Revision("revision", epoch, revision); _output->println();
    }

    /// <inheritdoc/>
    void OnStatePublicationStaleAcknowledgement(const ESPressio::State::DeviceIdentifier& device, ESPressio::State::StateTypeId typeId,
        ESPressio::State::StateEpoch epoch, ESPressio::State::StateRevision revision) override {
        Prefix("StaleAcknowledgement"); _output->print(" device="); Device(device); Type(typeId); Revision("revision", epoch, revision); _output->println();
    }
};

} // namespace ESPressio::Serial
