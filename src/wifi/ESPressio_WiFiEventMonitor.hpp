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
