#pragma once

#if !__has_include(<ESPressio_WiFi.hpp>)
#error "WiFiMonitor requires ESPressio WiFi >= 0.1.0 < 1.0.0."
#endif

#include <Arduino.h>
#include <ESPressio_WiFi.hpp>

namespace ESPressio::Serial {

class WiFiMonitor final : public ESPressio::WiFi::IWiFiObserver {
public:
    bool Initialize(Print& output, ESPressio::WiFi::WiFiManager& wifi) {
        if (_handle) return true;
        _output = &output;
        _handle = wifi.RegisterObserver(this);
        if (!_handle) { _output = nullptr; return false; }
        return true;
    }

    void Shutdown() { _handle.reset(); _output = nullptr; }
    bool IsInitialized() const noexcept { return static_cast<bool>(_handle); }

    void PrintStatus(const ESPressio::WiFi::WiFiManager& wifi) {
        if (!_output) return;
        const auto& s = wifi.State();
        _output->print("[ESPressio WiFi] Status mode="); _output->print(ModeName(s.Mode));
        _output->print(" ap="); _output->print(APStateName(s.AccessPoint.State));
        _output->print(" stations="); _output->print(static_cast<unsigned int>(s.AccessPoint.ConnectedStations));
        _output->print(" client="); _output->print(ClientStateName(s.Client.State));
        _output->print(" ip="); _output->print(s.Client.Network.Address.ToString().c_str());
        _output->print(" scan="); _output->println(ScanStateName(s.Scan));
    }

    void OnWiFiModeChanged(ESPressio::WiFi::WiFiMode before, ESPressio::WiFi::WiFiMode after) override {
        if (!_output) return; Prefix(); _output->print("Mode "); _output->print(ModeName(before)); _output->print(" -> "); _output->println(ModeName(after));
    }

    void OnClientStateChanged(const ESPressio::WiFi::ClientRuntimeState& before, const ESPressio::WiFi::ClientRuntimeState& after) override {
        if (!_output) return; Prefix(); _output->print("Client "); _output->print(ClientStateName(before.State)); _output->print(" -> "); _output->print(ClientStateName(after.State));
        if (!after.SSID.empty()) { _output->print(" ssid="); _output->print(after.SSID.c_str()); }
        if (after.State == ESPressio::WiFi::ClientState::Connected) { _output->print(" rssi="); _output->print(after.RSSI); _output->print(" channel="); _output->print(after.Channel); }
        _output->println();
    }

    void OnAccessPointStateChanged(const ESPressio::WiFi::AccessPointRuntimeState& before, const ESPressio::WiFi::AccessPointRuntimeState& after) override {
        if (!_output) return; Prefix(); _output->print("AP "); _output->print(APStateName(before.State)); _output->print(" -> "); _output->print(APStateName(after.State));
        if (!after.SSID.empty()) { _output->print(" ssid="); _output->print(after.SSID.c_str()); }
        _output->print(" stations="); _output->println(static_cast<unsigned int>(after.ConnectedStations));
    }

    void OnScanStateChanged(ESPressio::WiFi::ScanState before, ESPressio::WiFi::ScanState after) override {
        if (!_output) return; Prefix(); _output->print("Scan "); _output->print(ScanStateName(before)); _output->print(" -> "); _output->println(ScanStateName(after));
    }

    void OnScanCompleted(const std::vector<ESPressio::WiFi::ScanResult>& results) override {
        if (!_output) return; Prefix(); _output->print("ScanComplete count="); _output->println(static_cast<unsigned int>(results.size()));
        for (const auto& result : results) {
            _output->print("  ssid="); _output->print(result.SSID.c_str());
            _output->print(" rssi="); _output->print(result.RSSI);
            _output->print(" channel="); _output->print(result.Channel);
            _output->print(" security="); _output->println(SecurityName(result.Security));
        }
    }

    void OnAccessPointStationConnected(const ESPressio::WiFi::MacAddress& station) override { Station("APStationConnected", station); }
    void OnAccessPointStationDisconnected(const ESPressio::WiFi::MacAddress& station) override { Station("APStationDisconnected", station); }

    void OnClientIPAddressAcquired(const ESPressio::WiFi::NetworkAddress& network) override {
        if (!_output) return; Prefix(); _output->print("ClientIPAddressAcquired ip="); _output->print(network.Address.ToString().c_str());
        _output->print(" gateway="); _output->println(network.Gateway.ToString().c_str());
    }

    void OnClientIPAddressLost() override { if (_output) { Prefix(); _output->println("ClientIPAddressLost"); } }

private:
    void Prefix() { _output->print("[ESPressio WiFi] "); }
    void Station(const char* operation, const ESPressio::WiFi::MacAddress& mac) {
        if (!_output) return; Prefix(); _output->print(operation); _output->print(" station=");
        for (std::size_t i=0;i<mac.Octets.size();++i) { if (i) _output->print(':'); if (mac.Octets[i]<16) _output->print('0'); _output->print(mac.Octets[i], HEX); }
        _output->println();
    }
    static const char* ModeName(ESPressio::WiFi::WiFiMode v) { switch(v){case ESPressio::WiFi::WiFiMode::Disabled:return "disabled";case ESPressio::WiFi::WiFiMode::Client:return "client";case ESPressio::WiFi::WiFiMode::AccessPoint:return "ap";default:return "ap-client";} }
    static const char* ClientStateName(ESPressio::WiFi::ClientState v) { switch(v){case ESPressio::WiFi::ClientState::Disabled:return "disabled";case ESPressio::WiFi::ClientState::Idle:return "idle";case ESPressio::WiFi::ClientState::Connecting:return "connecting";case ESPressio::WiFi::ClientState::Connected:return "connected";case ESPressio::WiFi::ClientState::Reconnecting:return "reconnecting";default:return "failed";} }
    static const char* APStateName(ESPressio::WiFi::AccessPointState v) { switch(v){case ESPressio::WiFi::AccessPointState::Disabled:return "disabled";case ESPressio::WiFi::AccessPointState::Starting:return "starting";case ESPressio::WiFi::AccessPointState::Active:return "active";default:return "failed";} }
    static const char* ScanStateName(ESPressio::WiFi::ScanState v) { switch(v){case ESPressio::WiFi::ScanState::Idle:return "idle";case ESPressio::WiFi::ScanState::Scanning:return "scanning";case ESPressio::WiFi::ScanState::Complete:return "complete";default:return "failed";} }
    static const char* SecurityName(ESPressio::WiFi::NetworkSecurity v) { switch(v){case ESPressio::WiFi::NetworkSecurity::Open:return "open";case ESPressio::WiFi::NetworkSecurity::WEP:return "wep";case ESPressio::WiFi::NetworkSecurity::WPA:return "wpa";case ESPressio::WiFi::NetworkSecurity::WPA2:return "wpa2";case ESPressio::WiFi::NetworkSecurity::WPA_WPA2:return "wpa-wpa2";case ESPressio::WiFi::NetworkSecurity::WPA3:return "wpa3";case ESPressio::WiFi::NetworkSecurity::WPA2_WPA3:return "wpa2-wpa3";default:return "unknown";} }

    Print* _output = nullptr;
    ESPressio::Observable::ObserverHandlePtr _handle;
};

} // namespace ESPressio::Serial
