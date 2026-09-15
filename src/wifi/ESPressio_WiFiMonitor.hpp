#pragma once

#if !__has_include(<ESPressio_WiFi.hpp>)
#error "WiFiMonitor requires the final ESPressio WiFi runtime observer contract."
#endif

#include <Arduino.h>
#include <cstdio>
#include <ESPressio_WiFi.hpp>

namespace ESPressio::Serial {

/// <summary>Writes passive diagnostics from the final WiFi runtime observer/state surface.</summary>
/// <remarks>Uses WiFi's own observer contract directly; no family bridge, application worker, or transport shim is owned here.</remarks>
class WiFiMonitor final : public ESPressio::WiFi::IWiFiObserver {
private:
    Print* _output = nullptr;
    ESPressio::Observable::ObserverHandlePtr _handle;

    void Prefix() { _output->print("[ESPressio WiFi] "); }

    static const char* ModeName(ESPressio::WiFi::WiFiMode value) noexcept {
        switch (value) {
            case ESPressio::WiFi::WiFiMode::Disabled: return "disabled";
            case ESPressio::WiFi::WiFiMode::Client: return "client";
            case ESPressio::WiFi::WiFiMode::AccessPoint: return "ap";
            case ESPressio::WiFi::WiFiMode::AccessPointClient: return "ap-client";
            case ESPressio::WiFi::WiFiMode::APUntilClient: return "ap-until-client";
            case ESPressio::WiFi::WiFiMode::Off: return "off";
        }
        return "unknown";
    }

    static const char* ClientStateName(ESPressio::WiFi::ClientState value) noexcept {
        switch (value) {
            case ESPressio::WiFi::ClientState::Disabled: return "disabled";
            case ESPressio::WiFi::ClientState::Idle: return "idle";
            case ESPressio::WiFi::ClientState::Connecting: return "connecting";
            case ESPressio::WiFi::ClientState::Connected: return "connected";
            case ESPressio::WiFi::ClientState::Reconnecting: return "reconnecting";
            case ESPressio::WiFi::ClientState::Disconnecting: return "disconnecting";
            case ESPressio::WiFi::ClientState::Disconnected: return "disconnected";
            case ESPressio::WiFi::ClientState::Failed: return "failed";
        }
        return "unknown";
    }

    static const char* APStateName(ESPressio::WiFi::AccessPointState value) noexcept {
        switch (value) {
            case ESPressio::WiFi::AccessPointState::Disabled: return "disabled";
            case ESPressio::WiFi::AccessPointState::Starting: return "starting";
            case ESPressio::WiFi::AccessPointState::Active: return "active";
            case ESPressio::WiFi::AccessPointState::Failed: return "failed";
        }
        return "unknown";
    }

    static const char* ScanStateName(ESPressio::WiFi::ScanState value) noexcept {
        switch (value) {
            case ESPressio::WiFi::ScanState::Idle: return "idle";
            case ESPressio::WiFi::ScanState::Scanning: return "scanning";
            case ESPressio::WiFi::ScanState::Complete: return "complete";
            case ESPressio::WiFi::ScanState::Failed: return "failed";
        }
        return "unknown";
    }

    static const char* APUntilClientStateName(ESPressio::WiFi::APUntilClientState value) noexcept {
        switch (value) {
            case ESPressio::WiFi::APUntilClientState::Inactive: return "inactive";
            case ESPressio::WiFi::APUntilClientState::SeekingClient: return "seeking-client";
            case ESPressio::WiFi::APUntilClientState::FallbackAccessPoint: return "fallback-access-point";
            case ESPressio::WiFi::APUntilClientState::ClientConnected: return "client-connected";
        }
        return "unknown";
    }

    static const char* SelectionStateName(ESPressio::WiFi::ClientNetworkSelectionState value) noexcept {
        switch (value) {
            case ESPressio::WiFi::ClientNetworkSelectionState::Idle: return "idle";
            case ESPressio::WiFi::ClientNetworkSelectionState::Scanning: return "scanning";
            case ESPressio::WiFi::ClientNetworkSelectionState::Selecting: return "selecting";
            case ESPressio::WiFi::ClientNetworkSelectionState::Connecting: return "connecting";
            case ESPressio::WiFi::ClientNetworkSelectionState::Connected: return "connected";
            case ESPressio::WiFi::ClientNetworkSelectionState::NoKnownNetworkAvailable: return "no-known-network-available";
            case ESPressio::WiFi::ClientNetworkSelectionState::Exhausted: return "exhausted";
        }
        return "unknown";
    }

    static const char* SecurityName(ESPressio::WiFi::NetworkSecurity value) noexcept {
        switch (value) {
            case ESPressio::WiFi::NetworkSecurity::Open: return "open";
            case ESPressio::WiFi::NetworkSecurity::WEP: return "wep";
            case ESPressio::WiFi::NetworkSecurity::WPA: return "wpa";
            case ESPressio::WiFi::NetworkSecurity::WPA2: return "wpa2";
            case ESPressio::WiFi::NetworkSecurity::WPA_WPA2: return "wpa-wpa2";
            case ESPressio::WiFi::NetworkSecurity::WPA3: return "wpa3";
            case ESPressio::WiFi::NetworkSecurity::WPA2_WPA3: return "wpa2-wpa3";
            case ESPressio::WiFi::NetworkSecurity::Unknown: return "unknown";
        }
        return "unknown";
    }

    void PrintIPv4(const ESPressio::WiFi::IPv4Address& address) {
        const auto text = address.ToString();
        _output->print(text.c_str());
    }

    void PrintMac(const ESPressio::WiFi::MacAddress& address) {
        char text[18]{};
        std::snprintf(
            text,
            sizeof(text),
            "%02x:%02x:%02x:%02x:%02x:%02x",
            static_cast<unsigned>(address.Octets[0]),
            static_cast<unsigned>(address.Octets[1]),
            static_cast<unsigned>(address.Octets[2]),
            static_cast<unsigned>(address.Octets[3]),
            static_cast<unsigned>(address.Octets[4]),
            static_cast<unsigned>(address.Octets[5])
        );
        _output->print(text);
    }

public:
    bool Initialize(Print& output, ESPressio::WiFi::WiFiManager& wifi) {
        if (_handle) return true;
        _output = &output;
        _handle = wifi.RegisterObserver(this);
        if (!_handle) _output = nullptr;
        return static_cast<bool>(_handle);
    }

    void Shutdown() {
        _handle.reset();
        _output = nullptr;
    }

    bool IsInitialized() const noexcept { return static_cast<bool>(_handle); }

    bool PrintStatus(const ESPressio::WiFi::WiFiManager& wifi) {
        if (_output == nullptr) return false;
        const auto state = wifi.State();
        Prefix();
        _output->print("Status revision=");
        _output->print(static_cast<unsigned long long>(state.Revision));
        _output->print(" mode=");
        _output->print(ModeName(state.Mode));
        _output->print(" client=");
        _output->print(ClientStateName(state.Client.State));
        if (!state.Client.SSID.empty()) {
            _output->print(" ssid=");
            _output->print(state.Client.SSID.c_str());
        }
        _output->print(" ip=");
        PrintIPv4(state.Client.Network.Address);
        _output->print(" ap=");
        _output->print(APStateName(state.AccessPoint.State));
        _output->print(" scan=");
        _output->println(ScanStateName(state.Scan));
        return true;
    }

    void OnWiFiModeChanged(ESPressio::WiFi::WiFiMode before, ESPressio::WiFi::WiFiMode after) override {
        if (_output == nullptr) return;
        Prefix();
        _output->print("Mode ");
        _output->print(ModeName(before));
        _output->print(" -> ");
        _output->println(ModeName(after));
    }

    void OnClientStateChanged(
        const ESPressio::WiFi::ClientRuntimeState& before,
        const ESPressio::WiFi::ClientRuntimeState& after) override {
        if (_output == nullptr) return;
        Prefix();
        _output->print("Client ");
        _output->print(ClientStateName(before.State));
        _output->print(" -> ");
        _output->print(ClientStateName(after.State));
        if (!after.SSID.empty()) {
            _output->print(" ssid=");
            _output->print(after.SSID.c_str());
        }
        _output->print(" rssi=");
        _output->print(static_cast<long>(after.RSSI));
        _output->print(" channel=");
        _output->print(static_cast<unsigned long>(after.Channel));
        _output->print(" ip=");
        PrintIPv4(after.Network.Address);
        if (after.ReconnectAttempt != 0) {
            _output->print(" reconnect-attempt=");
            _output->print(static_cast<unsigned long>(after.ReconnectAttempt));
        }
        _output->println();
    }

    void OnAccessPointStateChanged(
        const ESPressio::WiFi::AccessPointRuntimeState& before,
        const ESPressio::WiFi::AccessPointRuntimeState& after) override {
        if (_output == nullptr) return;
        Prefix();
        _output->print("AP ");
        _output->print(APStateName(before.State));
        _output->print(" -> ");
        _output->print(APStateName(after.State));
        if (!after.SSID.empty()) {
            _output->print(" ssid=");
            _output->print(after.SSID.c_str());
        }
        _output->print(" channel=");
        _output->print(static_cast<unsigned long>(after.Channel));
        _output->print(" stations=");
        _output->println(static_cast<unsigned long>(after.ConnectedStations));
    }

    void OnAPUntilClientStateChanged(
        const ESPressio::WiFi::APUntilClientRuntimeState& before,
        const ESPressio::WiFi::APUntilClientRuntimeState& after) override {
        if (_output == nullptr) return;
        Prefix();
        _output->print("APUntilClient ");
        _output->print(APUntilClientStateName(before.State));
        _output->print(" -> ");
        _output->print(APUntilClientStateName(after.State));
        _output->print(" fallback-active=");
        _output->print(after.FallbackAccessPointActive ? "true" : "false");
        _output->print(" next-retry-ms=");
        _output->println(static_cast<unsigned long long>(after.NextRetryMilliseconds));
    }

    void OnScanStateChanged(ESPressio::WiFi::ScanState before, ESPressio::WiFi::ScanState after) override {
        if (_output == nullptr) return;
        Prefix();
        _output->print("Scan ");
        _output->print(ScanStateName(before));
        _output->print(" -> ");
        _output->println(ScanStateName(after));
    }

    void OnScanCompleted(const ESPressio::WiFi::WiFiVector<ESPressio::WiFi::ScanResult>& results) override {
        if (_output == nullptr) return;
        Prefix();
        _output->print("ScanComplete count=");
        _output->println(static_cast<unsigned long>(results.size()));
        for (const auto& result : results) {
            Prefix();
            _output->print("ScanResult ssid=");
            _output->print(result.SSID.c_str());
            _output->print(" rssi=");
            _output->print(static_cast<long>(result.RSSI));
            _output->print(" channel=");
            _output->print(static_cast<unsigned long>(result.Channel));
            _output->print(" security=");
            _output->println(SecurityName(result.Security));
        }
    }

    void OnAccessPointStationConnected(const ESPressio::WiFi::MacAddress& station) override {
        if (_output == nullptr) return;
        Prefix();
        _output->print("APStationConnected mac=");
        PrintMac(station);
        _output->println();
    }

    void OnAccessPointStationDisconnected(const ESPressio::WiFi::MacAddress& station) override {
        if (_output == nullptr) return;
        Prefix();
        _output->print("APStationDisconnected mac=");
        PrintMac(station);
        _output->println();
    }

    void OnClientIPAddressAcquired(const ESPressio::WiFi::NetworkAddress& network) override {
        if (_output == nullptr) return;
        Prefix();
        _output->print("ClientIPAddressAcquired address=");
        PrintIPv4(network.Address);
        _output->println();
    }

    void OnClientIPAddressLost() override {
        if (_output == nullptr) return;
        Prefix();
        _output->println("ClientIPAddressLost");
    }

    void OnClientNetworkSelectionChanged(
        const ESPressio::WiFi::ClientNetworkSelectionRuntimeState& before,
        const ESPressio::WiFi::ClientNetworkSelectionRuntimeState& after) override {
        if (_output == nullptr) return;
        Prefix();
        _output->print("Selection ");
        _output->print(SelectionStateName(before.State));
        _output->print(" -> ");
        _output->print(SelectionStateName(after.State));
        if (!after.SelectedSSID.empty()) {
            _output->print(" selected=");
            _output->print(after.SelectedSSID.c_str());
        }
        _output->print(" priority=");
        _output->print(static_cast<unsigned long>(after.SelectedPriority));
        _output->print(" candidates=");
        _output->println(static_cast<unsigned long>(after.EligibleCandidateCount));
    }

    void OnClientNetworkSelected(const ESPressio::WiFi::ClientNetworkCandidate& selected) override {
        if (_output == nullptr) return;
        Prefix();
        _output->print("NetworkSelected ssid=");
        _output->print(selected.SSID.c_str());
        _output->print(" priority=");
        _output->print(static_cast<unsigned long>(selected.Priority));
        _output->print(" rssi=");
        _output->print(static_cast<long>(selected.RSSI));
        _output->print(" channel=");
        _output->print(static_cast<unsigned long>(selected.Channel));
        _output->print(" bssid=");
        PrintMac(selected.BSSID);
        _output->println();
    }

    void OnClientNoKnownNetworkAvailable() override {
        if (_output == nullptr) return;
        Prefix();
        _output->println("NoKnownNetworkAvailable");
    }
};

} // namespace ESPressio::Serial
