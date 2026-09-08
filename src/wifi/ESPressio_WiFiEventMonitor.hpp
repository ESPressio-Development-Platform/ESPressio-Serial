#pragma once

#if !__has_include(<ESPressio_EventThread.hpp>) || !__has_include(<ESPressio_WiFiEvents.hpp>)
#error "WiFiEventMonitor requires ESPressio Event >= 6.0.3 and ESPressio WiFi >= 0.2.0."
#endif

#include <Arduino.h>
#include <ESPressio_EventThread.hpp>
#include <ESPressio_Memory.hpp>
#include <ESPressio_WiFiEvents.hpp>

namespace ESPressio::Serial {

/// <summary>Observes Wi-Fi Events and writes concise diagnostics to an Arduino Print sink.</summary>
/// <remarks>The retained listener-handle table uses ESPressio System ExternalPreferred storage so optional diagnostics do not consume scarce internal DRAM for long-lived registry capacity.</remarks>
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 532 bytes [EventThread: EventThreadBase: Thread: _taskExited: owned object: 4 bytes; EventThread: EventThreadBase: Thread: _taskStartGate: owned object: 4 bytes; EventThread: EventThreadBase: Thread: _taskConfigurationMutex: _owned: owned object: 4 bytes; EventThread: EventThreadBase: Thread: _taskConfigurationMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventThreadBase: Thread: _stateTransitionMutex: _owned: owned object: 4 bytes; EventThread: EventThreadBase: Thread: _stateTransitionMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventThreadBase: Thread: _lifecycleObservable: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 96 bytes; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 20 bytes; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: pointee: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: pointee: _condition: native condition-variable state may allocate platform synchronization resources; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: Observable: _registrations: Capacity * (12 bytes) element storage; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: Observable: _bindings: Capacity * (12 bytes) element storage; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: _mutex: _owned: owned object: 4 bytes; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: _mutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: _notificationMutex: _owned: owned object: 4 bytes; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: _notificationMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventThreadBase: Thread: _callbackMutex: _owned: owned object: 4 bytes; EventThread: EventThreadBase: Thread: _callbackMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventThreadBase: Thread: _onDestroy: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onInitialize: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onStart: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onPause: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onTerminate: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onTerminated: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onInitializationFailed: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onExecutionFailed: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onStateChange: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: EventReceiver: _eventsMutex: _owned: owned object: 4 bytes; EventThread: EventThreadBase: EventReceiver: _eventsMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventThreadBase: EventReceiver: _capacityAvailable: owned object: 4 bytes; EventThread: EventThreadBase: EventReceiver: _priorityQueues: 3 elements each: Capacity * (16 bytes) element storage; EventThread: EventThreadBase: EventReceiver: _priorityStacks: 3 elements each: Capacity * (16 bytes) element storage; EventThread: EventThreadBase: _eventSignal: owned object: 4 bytes; EventThread: EventThreadBase: _eventSignalMutex: _owned: owned object: 4 bytes; EventThread: EventThreadBase: _eventSignalMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventListener: _listeners: implementation blocks containing N * (40 bytes) plus block-map pointers; EventThread: EventListener: _listeners: N elements each: Callback: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventListener: _listeners: N elements each: CustomInterest: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventListener: _listenersMutex: _owned: owned object: 4 bytes; EventThread: EventListener: _listenersMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily]
 * Requires Stack/Heap Preallocation
 * Members:
 * - _output (Print&): 4 bytes [0 bytes dynamic allocation]
 * - _handles (ListenerHandleStorage): 12 bytes [Capacity * (12 bytes) element storage; N live elements each: owned object: 4 bytes]
 * - _listenersRegistered (bool): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: 552 bytes [EventThread: EventThreadBase: Thread: _taskExited: owned object: 4 bytes; EventThread: EventThreadBase: Thread: _taskStartGate: owned object: 4 bytes; EventThread: EventThreadBase: Thread: _taskConfigurationMutex: _owned: owned object: 4 bytes; EventThread: EventThreadBase: Thread: _taskConfigurationMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventThreadBase: Thread: _stateTransitionMutex: _owned: owned object: 4 bytes; EventThread: EventThreadBase: Thread: _stateTransitionMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventThreadBase: Thread: _lifecycleObservable: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 96 bytes; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 20 bytes; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: pointee: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: pointee: _condition: native condition-variable state may allocate platform synchronization resources; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: Observable: _registrations: Capacity * (12 bytes) element storage; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: Observable: _bindings: Capacity * (12 bytes) element storage; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: _mutex: _owned: owned object: 4 bytes; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: _mutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: _notificationMutex: _owned: owned object: 4 bytes; EventThread: EventThreadBase: Thread: _lifecycleObservable: pointee: ThreadSafeObservable: _notificationMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventThreadBase: Thread: _callbackMutex: _owned: owned object: 4 bytes; EventThread: EventThreadBase: Thread: _callbackMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventThreadBase: Thread: _onDestroy: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onInitialize: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onStart: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onPause: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onTerminate: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onTerminated: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onInitializationFailed: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onExecutionFailed: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: Thread: _onStateChange: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventThreadBase: EventReceiver: _eventsMutex: _owned: owned object: 4 bytes; EventThread: EventThreadBase: EventReceiver: _eventsMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventThreadBase: EventReceiver: _capacityAvailable: owned object: 4 bytes; EventThread: EventThreadBase: EventReceiver: _priorityQueues: 3 elements each: Capacity * (16 bytes) element storage; EventThread: EventThreadBase: EventReceiver: _priorityStacks: 3 elements each: Capacity * (16 bytes) element storage; EventThread: EventThreadBase: _eventSignal: owned object: 4 bytes; EventThread: EventThreadBase: _eventSignalMutex: _owned: owned object: 4 bytes; EventThread: EventThreadBase: _eventSignalMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; EventThread: EventListener: _listeners: implementation blocks containing N * (40 bytes) plus block-map pointers; EventThread: EventListener: _listeners: N elements each: Callback: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventListener: _listeners: N elements each: CustomInterest: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 4 bytes; EventThread: EventListener: _listenersMutex: _owned: owned object: 4 bytes; EventThread: EventListener: _listenersMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; _handles: Capacity * (12 bytes) element storage; _handles: N live elements each: owned object: 4 bytes]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class WiFiEventMonitor final : public Event::EventThread {
public:
    explicit WiFiEventMonitor(Print& output)
        : Event::EventThread(
              Threads::ThreadReleasePolicy::ExplicitRelease
          ),
          _output(output) {
        SetStartOnInitialize(false);
    }

    Threads::ThreadInitializationStatus Initialize() {
        if (!_listenersRegistered) {
            RegisterAll();
            _listenersRegistered = true;
        }

        return Event::EventThread::Initialize();
    }

private:
    static constexpr auto ExternalPreferred =
        System::Memory::MemoryPolicy::ExternalPreferred;
    using ListenerHandleStorage = System::Memory::Vector<
        Event::EventListenerHandlePtr,
        ExternalPreferred
    >;

    Print& _output;
    ListenerHandleStorage _handles;
    bool _listenersRegistered = false;

    void Prefix() { _output.print("[ESPressio WiFi Event] "); }

    static char Hex(uint8_t nibble) {
        return static_cast<char>(nibble < 10 ? '0' + nibble : 'A' + (nibble - 10));
    }

    void PrintMac(const WiFi::MacAddress& mac) {
        for (std::size_t i = 0; i < mac.Octets.size(); ++i) {
            if (i) _output.print(':');
            const auto value = mac.Octets[i];
            _output.print(Hex(value >> 4));
            _output.print(Hex(value & 0x0F));
        }
    }

    static const char* ModeName(WiFi::WiFiMode value) {
        switch (value) {
            case WiFi::WiFiMode::Disabled: return "disabled";
            case WiFi::WiFiMode::Client: return "client";
            case WiFi::WiFiMode::AccessPoint: return "ap";
            case WiFi::WiFiMode::AccessPointClient: return "ap-client";
            case WiFi::WiFiMode::APUntilClient: return "ap-until-client";
            case WiFi::WiFiMode::Off: return "off";
        }
        return "unknown";
    }

    static const char* ClientStateName(WiFi::ClientState value) {
        switch (value) {
            case WiFi::ClientState::Disabled: return "disabled";
            case WiFi::ClientState::Idle: return "idle";
            case WiFi::ClientState::Connecting: return "connecting";
            case WiFi::ClientState::Connected: return "connected";
            case WiFi::ClientState::Reconnecting: return "reconnecting";
            case WiFi::ClientState::Disconnecting: return "disconnecting";
            case WiFi::ClientState::Disconnected: return "disconnected";
            case WiFi::ClientState::Failed: return "failed";
        }
        return "unknown";
    }

    static const char* APStateName(WiFi::AccessPointState value) {
        switch (value) {
            case WiFi::AccessPointState::Disabled: return "disabled";
            case WiFi::AccessPointState::Starting: return "starting";
            case WiFi::AccessPointState::Active: return "active";
            case WiFi::AccessPointState::Failed: return "failed";
        }
        return "unknown";
    }

    static const char* ScanStateName(WiFi::ScanState value) {
        switch (value) {
            case WiFi::ScanState::Idle: return "idle";
            case WiFi::ScanState::Scanning: return "scanning";
            case WiFi::ScanState::Complete: return "complete";
            case WiFi::ScanState::Failed: return "failed";
        }
        return "unknown";
    }

    static const char* SelectionStateName(WiFi::ClientNetworkSelectionState value) {
        switch (value) {
            case WiFi::ClientNetworkSelectionState::Idle: return "idle";
            case WiFi::ClientNetworkSelectionState::Scanning: return "scanning";
            case WiFi::ClientNetworkSelectionState::Selecting: return "selecting";
            case WiFi::ClientNetworkSelectionState::Connecting: return "connecting";
            case WiFi::ClientNetworkSelectionState::Connected: return "connected";
            case WiFi::ClientNetworkSelectionState::NoKnownNetworkAvailable: return "no-known-network-available";
            case WiFi::ClientNetworkSelectionState::Exhausted: return "exhausted";
        }
        return "unknown";
    }

    static const char* APUntilClientStateName(WiFi::APUntilClientState value) {
        switch (value) {
            case WiFi::APUntilClientState::Inactive: return "inactive";
            case WiFi::APUntilClientState::SeekingClient: return "seeking-client";
            case WiFi::APUntilClientState::FallbackAccessPoint: return "fallback-access-point";
            case WiFi::APUntilClientState::ClientConnected: return "client-connected";
        }
        return "unknown";
    }

    void RegisterAll() {
        _handles.push_back(RegisterListener<Event::WiFiModeChangedEvent>([this](auto* event, auto, auto) {
            Prefix(); _output.print("Mode "); _output.print(ModeName(event->Before)); _output.print(" -> "); _output.println(ModeName(event->After));
        }));

        _handles.push_back(RegisterListener<Event::WiFiClientStateChangedEvent>([this](auto* event, auto, auto) {
            Prefix(); _output.print("Client "); _output.print(ClientStateName(event->Before)); _output.print(" -> "); _output.print(ClientStateName(event->After));
            if (!event->SSID.empty()) { _output.print(" ssid="); _output.print(event->SSID.c_str()); }
            _output.print(" ip="); _output.println(event->Network.Address.ToString().c_str());
        }));

        _handles.push_back(RegisterListener<Event::WiFiAccessPointStateChangedEvent>([this](auto* event, auto, auto) {
            Prefix(); _output.print("AP "); _output.print(APStateName(event->Before)); _output.print(" -> "); _output.print(APStateName(event->After));
            if (!event->SSID.empty()) { _output.print(" ssid="); _output.print(event->SSID.c_str()); }
            _output.print(" stations="); _output.println(static_cast<unsigned int>(event->ConnectedStations));
        }));

        _handles.push_back(RegisterListener<Event::WiFiAPUntilClientStateChangedEvent>([this](auto* event, auto, auto) {
            Prefix(); _output.print("APUntilClient "); _output.print(APUntilClientStateName(event->Before)); _output.print(" -> "); _output.print(APUntilClientStateName(event->After));
            _output.print(" fallback-ap="); _output.println(event->FallbackAccessPointActive ? "active" : "inactive");
        }));

        _handles.push_back(RegisterListener<Event::WiFiScanStateChangedEvent>([this](auto* event, auto, auto) {
            Prefix(); _output.print("Scan "); _output.print(ScanStateName(event->Before)); _output.print(" -> "); _output.println(ScanStateName(event->After));
        }));

        _handles.push_back(RegisterListener<Event::WiFiScanCompletedEvent>([this](auto* event, auto, auto) {
            Prefix(); _output.print("ScanComplete count="); _output.println(static_cast<unsigned int>(event->Results.size()));
            for (const auto& result : event->Results) {
                _output.print("  ssid="); _output.print(result.SSID.c_str());
                _output.print(" rssi="); _output.print(result.RSSI);
                _output.print(" channel="); _output.println(static_cast<unsigned int>(result.Channel));
            }
        }));

        _handles.push_back(RegisterListener<Event::WiFiAccessPointStationConnectedEvent>([this](auto* event, auto, auto) {
            Prefix(); _output.print("APStationConnected station="); PrintMac(event->Station); _output.println();
        }));

        _handles.push_back(RegisterListener<Event::WiFiAccessPointStationDisconnectedEvent>([this](auto* event, auto, auto) {
            Prefix(); _output.print("APStationDisconnected station="); PrintMac(event->Station); _output.println();
        }));

        _handles.push_back(RegisterListener<Event::WiFiClientIPAddressAcquiredEvent>([this](auto* event, auto, auto) {
            Prefix(); _output.print("ClientIPAddressAcquired ip="); _output.print(event->Network.Address.ToString().c_str());
            _output.print(" gateway="); _output.println(event->Network.Gateway.ToString().c_str());
        }));

        _handles.push_back(RegisterListener<Event::WiFiClientIPAddressLostEvent>([this](auto*, auto, auto) {
            Prefix(); _output.println("ClientIPAddressLost");
        }));

        _handles.push_back(RegisterListener<Event::WiFiClientNetworkSelectionChangedEvent>([this](auto* event, auto, auto) {
            Prefix(); _output.print("Selection "); _output.print(SelectionStateName(event->Before)); _output.print(" -> "); _output.print(SelectionStateName(event->After));
            if (!event->SelectedSSID.empty()) { _output.print(" selected="); _output.print(event->SelectedSSID.c_str()); }
            _output.print(" priority="); _output.print(static_cast<unsigned int>(event->SelectedPriority));
            _output.print(" candidates="); _output.println(static_cast<unsigned long>(event->EligibleCandidateCount));
        }));

        _handles.push_back(RegisterListener<Event::WiFiClientNetworkSelectedEvent>([this](auto* event, auto, auto) {
            Prefix(); _output.print("SelectedNetwork ssid="); _output.print(event->SSID.c_str());
            _output.print(" bssid="); PrintMac(event->BSSID);
            _output.print(" priority="); _output.print(static_cast<unsigned int>(event->Priority));
            _output.print(" rssi="); _output.print(event->RSSI);
            _output.print(" channel="); _output.println(static_cast<unsigned int>(event->Channel));
        }));

        _handles.push_back(RegisterListener<Event::WiFiClientNoKnownNetworkAvailableEvent>([this](auto*, auto, auto) {
            Prefix(); _output.println("NoKnownNetworkAvailable");
        }));
    }
};

} // namespace ESPressio::Serial
