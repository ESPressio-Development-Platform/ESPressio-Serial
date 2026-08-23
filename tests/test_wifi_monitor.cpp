#include <cassert>
#include <string>
#include <vector>
#include <ESPressio_WiFiMonitor.hpp>

using namespace ESPressio;

class BufferPrint final : public Print {
public:
    std::size_t write(uint8_t value) override { text.push_back(static_cast<char>(value)); return 1; }
    std::string text;
};

class FakePlatform final : public WiFi::IWiFiPlatform {
public:
    WiFi::WiFiStatus Apply(const WiFi::WiFiConfiguration& config) override { state.Mode=config.Mode; state.AccessPoint.SSID=config.AccessPoint.SSID; state.Revision++; return WiFi::WiFiStatus::Success; }
    WiFi::WiFiStatus Disable() override { return WiFi::WiFiStatus::Success; }
    WiFi::WiFiStatus ConnectClient() override { return WiFi::WiFiStatus::Success; }
    WiFi::WiFiStatus DisconnectClient() override { return WiFi::WiFiStatus::Success; }
    WiFi::WiFiStatus StartAccessPoint() override { return WiFi::WiFiStatus::Success; }
    WiFi::WiFiStatus StopAccessPoint() override { return WiFi::WiFiStatus::Success; }
    WiFi::WiFiStatus StartScan() override { return WiFi::WiFiStatus::Success; }
    WiFi::WiFiStatus Poll(WiFi::WiFiRuntimeState& output,std::vector<WiFi::ScanResult>* scans,std::vector<WiFi::WiFiPlatformEvent>* events) override {
        output=state; if(scans && scanReady){WiFi::ScanResult r;r.SSID="VisibleNetwork";r.RSSI=-50;r.Channel=11;r.Security=WiFi::NetworkSecurity::WPA2;scans->push_back(r);scanReady=false;} if(events){*events=pending;pending.clear();} return WiFi::WiFiStatus::Success;
    }
    WiFi::WiFiRuntimeState state{}; bool scanReady=false; std::vector<WiFi::WiFiPlatformEvent> pending;
};

int main(){
    FakePlatform platform; WiFi::WiFiManager wifi(platform); BufferPrint output; Serial::WiFiMonitor monitor;
    assert(monitor.Initialize(output,wifi));

    WiFi::WiFiConfiguration config; config.Mode=WiFi::WiFiMode::AccessPointClient; config.AccessPoint.SSID="LabAP"; config.AccessPoint.Password="DO-NOT-PRINT"; config.Client.Password="ALSO-SECRET";
    assert(wifi.Configure(config)==WiFi::WiFiStatus::Success); assert(wifi.Poll()==WiFi::WiFiStatus::Success);

    platform.state.Client.State=WiFi::ClientState::Connected; platform.state.Client.SSID="Studio"; platform.state.Client.RSSI=-43; platform.state.Client.Channel=6; platform.state.Client.Network.Address=WiFi::IPv4Address(192,168,1,42); platform.state.Revision++;
    assert(wifi.Poll()==WiFi::WiFiStatus::Success);

    platform.state.Client.State=WiFi::ClientState::Reconnecting; platform.state.Client.ReconnectAttempt=2; platform.state.Revision++;
    assert(wifi.Poll()==WiFi::WiFiStatus::Success);
    platform.state.Client.State=WiFi::ClientState::Disconnecting; platform.state.Revision++;
    assert(wifi.Poll()==WiFi::WiFiStatus::Success);
    platform.state.Client.State=WiFi::ClientState::Disconnected; platform.state.Revision++;
    assert(wifi.Poll()==WiFi::WiFiStatus::Success);

    platform.scanReady=true; platform.state.Scan=WiFi::ScanState::Complete; platform.state.Revision++; assert(wifi.Poll()==WiFi::WiFiStatus::Success);
    monitor.PrintStatus(wifi);

    assert(output.text.find("[ESPressio WiFi]")!=std::string::npos);
    assert(output.text.find("Studio")!=std::string::npos);
    assert(output.text.find("VisibleNetwork")!=std::string::npos);
    assert(output.text.find("192.168.1.42")!=std::string::npos);
    assert(output.text.find("reconnect-attempt=2")!=std::string::npos);
    assert(output.text.find("disconnecting")!=std::string::npos);
    assert(output.text.find("disconnected")!=std::string::npos);
    assert(output.text.find("DO-NOT-PRINT")==std::string::npos);
    assert(output.text.find("ALSO-SECRET")==std::string::npos);
    monitor.Shutdown(); assert(!monitor.IsInitialized());
    return 0;
}
