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
        _output=&output; _handle=wifi.RegisterObserver(this);
        if (!_handle) { _output=nullptr; return false; }
        return true;
    }
    void Shutdown() { _handle.reset(); _output=nullptr; }
    bool IsInitialized() const noexcept { return static_cast<bool>(_handle); }

    void PrintStatus(const ESPressio::WiFi::WiFiManager& wifi) {
        if (!_output) return; const auto& s=wifi.State();
        Prefix(); _output->print("Status mode="); _output->print(ModeName(s.Mode));
        _output->print(" ap="); _output->print(APStateName(s.AccessPoint.State));
        _output->print(" stations="); _output->print(static_cast<unsigned int>(s.AccessPoint.ConnectedStations));
        _output->print(" client="); _output->print(ClientStateName(s.Client.State));
        _output->print(" ip="); _output->print(s.Client.Network.Address.ToString().c_str());
        _output->print(" scan="); _output->print(ScanStateName(s.Scan));
        _output->print(" selection="); _output->print(SelectionStateName(s.Client.Selection.State));
        _output->print(" ap-until-client="); _output->println(APUntilClientStateName(s.APUntilClient.State));
    }

    void OnWiFiModeChanged(ESPressio::WiFi::WiFiMode before, ESPressio::WiFi::WiFiMode after) override {
        if(!_output)return; Prefix(); _output->print("Mode "); _output->print(ModeName(before)); _output->print(" -> "); _output->println(ModeName(after));
    }
    void OnClientStateChanged(const ESPressio::WiFi::ClientRuntimeState& before,const ESPressio::WiFi::ClientRuntimeState& after) override {
        if(!_output)return; Prefix(); _output->print("Client "); _output->print(ClientStateName(before.State)); _output->print(" -> "); _output->print(ClientStateName(after.State));
        if(!after.SSID.empty()){_output->print(" ssid=");_output->print(after.SSID.c_str());}
        if(after.State==ESPressio::WiFi::ClientState::Connected){_output->print(" rssi=");_output->print(after.RSSI);_output->print(" channel=");_output->print(static_cast<unsigned int>(after.Channel));}
        if(after.ReconnectAttempt){_output->print(" reconnect-attempt=");_output->print(static_cast<unsigned long>(after.ReconnectAttempt));}
        _output->println();
    }
    void OnAccessPointStateChanged(const ESPressio::WiFi::AccessPointRuntimeState& before,const ESPressio::WiFi::AccessPointRuntimeState& after) override {
        if(!_output)return; Prefix(); _output->print("AP ");_output->print(APStateName(before.State));_output->print(" -> ");_output->print(APStateName(after.State));
        if(!after.SSID.empty()){_output->print(" ssid=");_output->print(after.SSID.c_str());}
        _output->print(" stations=");_output->println(static_cast<unsigned int>(after.ConnectedStations));
    }
    void OnAPUntilClientStateChanged(const ESPressio::WiFi::APUntilClientRuntimeState& before,const ESPressio::WiFi::APUntilClientRuntimeState& after) override {
        if(!_output)return; Prefix(); _output->print("APUntilClient "); _output->print(APUntilClientStateName(before.State)); _output->print(" -> "); _output->print(APUntilClientStateName(after.State));
        _output->print(" fallback-ap="); _output->print(after.FallbackAccessPointActive?"true":"false");
        _output->print(" fallback-deadline-ms="); _output->print(static_cast<unsigned long long>(after.FallbackDeadlineMilliseconds));
        _output->print(" next-retry-ms="); _output->println(static_cast<unsigned long long>(after.NextRetryMilliseconds));
    }
    void OnScanStateChanged(ESPressio::WiFi::ScanState before,ESPressio::WiFi::ScanState after) override {
        if(!_output)return;Prefix();_output->print("Scan ");_output->print(ScanStateName(before));_output->print(" -> ");_output->println(ScanStateName(after));
    }
    void OnScanCompleted(const std::vector<ESPressio::WiFi::ScanResult>& results) override {
        if(!_output)return;Prefix();_output->print("ScanComplete count=");_output->println(static_cast<unsigned int>(results.size()));
        for(const auto& r:results){_output->print("  ssid=");_output->print(r.SSID.c_str());_output->print(" rssi=");_output->print(r.RSSI);_output->print(" channel=");_output->print(static_cast<unsigned int>(r.Channel));_output->print(" security=");_output->println(SecurityName(r.Security));}
    }
    void OnAccessPointStationConnected(const ESPressio::WiFi::MacAddress& station) override { Station("APStationConnected",station); }
    void OnAccessPointStationDisconnected(const ESPressio::WiFi::MacAddress& station) override { Station("APStationDisconnected",station); }
    void OnClientIPAddressAcquired(const ESPressio::WiFi::NetworkAddress& n) override {
        if(!_output)return;Prefix();_output->print("ClientIPAddressAcquired ip=");_output->print(n.Address.ToString().c_str());_output->print(" gateway=");_output->println(n.Gateway.ToString().c_str());
    }
    void OnClientIPAddressLost() override { if(_output){Prefix();_output->println("ClientIPAddressLost");} }
    void OnClientNetworkSelectionChanged(const ESPressio::WiFi::ClientNetworkSelectionRuntimeState& before,const ESPressio::WiFi::ClientNetworkSelectionRuntimeState& after) override {
        if(!_output)return; Prefix(); _output->print("Selection "); _output->print(SelectionStateName(before.State)); _output->print(" -> "); _output->print(SelectionStateName(after.State));
        if(!after.SelectedSSID.empty()){_output->print(" selected=");_output->print(after.SelectedSSID.c_str());_output->print(" priority=");_output->print(static_cast<unsigned int>(after.SelectedPriority));}
        _output->print(" candidates="); _output->println(static_cast<unsigned int>(after.EligibleCandidateCount));
    }
    void OnClientNetworkSelected(const ESPressio::WiFi::ClientNetworkCandidate& selected) override {
        if(!_output)return; Prefix(); _output->print("NetworkSelected ssid=");_output->print(selected.SSID.c_str());_output->print(" priority=");_output->print(static_cast<unsigned int>(selected.Priority));_output->print(" rssi=");_output->print(selected.RSSI);_output->print(" channel=");_output->print(static_cast<unsigned int>(selected.Channel));_output->print(" bssid=");PrintMac(selected.BSSID);_output->println();
    }
    void OnClientNoKnownNetworkAvailable() override { if(_output){Prefix();_output->println("NoKnownNetworkAvailable");} }

private:
    void Prefix(){_output->print("[ESPressio WiFi] ");}
    static char Hex(uint8_t nibble){return static_cast<char>(nibble<10?'0'+nibble:'A'+(nibble-10));}
    void PrintMac(const ESPressio::WiFi::MacAddress& mac){for(std::size_t i=0;i<mac.Octets.size();++i){if(i)_output->print(':');const uint8_t v=mac.Octets[i];_output->print(Hex(v>>4));_output->print(Hex(v&0x0F));}}
    void Station(const char* operation,const ESPressio::WiFi::MacAddress& mac){
        if(!_output)return;Prefix();_output->print(operation);_output->print(" station=");PrintMac(mac);_output->println();
    }
    static const char* ModeName(ESPressio::WiFi::WiFiMode v){switch(v){case ESPressio::WiFi::WiFiMode::Disabled:return"disabled";case ESPressio::WiFi::WiFiMode::Client:return"client";case ESPressio::WiFi::WiFiMode::AccessPoint:return"ap";case ESPressio::WiFi::WiFiMode::AccessPointClient:return"ap-client";case ESPressio::WiFi::WiFiMode::APUntilClient:return"ap-until-client";default:return"unknown";}}
    static const char* ClientStateName(ESPressio::WiFi::ClientState v){switch(v){case ESPressio::WiFi::ClientState::Disabled:return"disabled";case ESPressio::WiFi::ClientState::Idle:return"idle";case ESPressio::WiFi::ClientState::Connecting:return"connecting";case ESPressio::WiFi::ClientState::Connected:return"connected";case ESPressio::WiFi::ClientState::Reconnecting:return"reconnecting";case ESPressio::WiFi::ClientState::Disconnecting:return"disconnecting";case ESPressio::WiFi::ClientState::Disconnected:return"disconnected";case ESPressio::WiFi::ClientState::Failed:return"failed";default:return"unknown";}}
    static const char* APStateName(ESPressio::WiFi::AccessPointState v){switch(v){case ESPressio::WiFi::AccessPointState::Disabled:return"disabled";case ESPressio::WiFi::AccessPointState::Starting:return"starting";case ESPressio::WiFi::AccessPointState::Active:return"active";case ESPressio::WiFi::AccessPointState::Failed:return"failed";default:return"unknown";}}
    static const char* APUntilClientStateName(ESPressio::WiFi::APUntilClientState v){switch(v){case ESPressio::WiFi::APUntilClientState::Inactive:return"inactive";case ESPressio::WiFi::APUntilClientState::SeekingClient:return"seeking-client";case ESPressio::WiFi::APUntilClientState::FallbackAccessPoint:return"fallback-access-point";case ESPressio::WiFi::APUntilClientState::ClientConnected:return"client-connected";default:return"unknown";}}
    static const char* ScanStateName(ESPressio::WiFi::ScanState v){switch(v){case ESPressio::WiFi::ScanState::Idle:return"idle";case ESPressio::WiFi::ScanState::Scanning:return"scanning";case ESPressio::WiFi::ScanState::Complete:return"complete";case ESPressio::WiFi::ScanState::Failed:return"failed";default:return"unknown";}}
    static const char* SelectionStateName(ESPressio::WiFi::ClientNetworkSelectionState v){switch(v){case ESPressio::WiFi::ClientNetworkSelectionState::Idle:return"idle";case ESPressio::WiFi::ClientNetworkSelectionState::Scanning:return"scanning";case ESPressio::WiFi::ClientNetworkSelectionState::Selecting:return"selecting";case ESPressio::WiFi::ClientNetworkSelectionState::Connecting:return"connecting";case ESPressio::WiFi::ClientNetworkSelectionState::Connected:return"connected";case ESPressio::WiFi::ClientNetworkSelectionState::NoKnownNetworkAvailable:return"no-known-network-available";case ESPressio::WiFi::ClientNetworkSelectionState::Exhausted:return"exhausted";default:return"unknown";}}
    static const char* SecurityName(ESPressio::WiFi::NetworkSecurity v){switch(v){case ESPressio::WiFi::NetworkSecurity::Open:return"open";case ESPressio::WiFi::NetworkSecurity::WEP:return"wep";case ESPressio::WiFi::NetworkSecurity::WPA:return"wpa";case ESPressio::WiFi::NetworkSecurity::WPA2:return"wpa2";case ESPressio::WiFi::NetworkSecurity::WPA_WPA2:return"wpa-wpa2";case ESPressio::WiFi::NetworkSecurity::WPA3:return"wpa3";case ESPressio::WiFi::NetworkSecurity::WPA2_WPA3:return"wpa2-wpa3";case ESPressio::WiFi::NetworkSecurity::Unknown:return"unknown";default:return"unknown";}}
    Print* _output=nullptr;
    ESPressio::Observable::ObserverHandlePtr _handle;
};

} // namespace ESPressio::Serial
