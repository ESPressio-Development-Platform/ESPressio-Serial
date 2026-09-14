#pragma once

#if !__has_include(<ESPressio_WiFi.hpp>)
#error "WiFiMonitor requires the final ESPressio WiFi runtime observer contract."
#endif

#include <Arduino.h>
#include <ESPressio_WiFi.hpp>

namespace ESPressio::Serial {

/// <summary>Writes passive diagnostics from the final WiFi runtime observer/state surface.</summary>
/// <remarks>Uses WiFi's own observer contract directly; no Event bridge, EventThread, worker, or transport shim is owned here.</remarks>
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
        const auto& state = wifi.State();
        Prefix();
        _output->print("Status revision=");
        _output->print(static_cast<unsigned long long>(state.Revision));
        _output->print(" mode=");
        _output->print(ModeName(state.Mode));
        _output->print(" client=");
        _output->print(ClientStateName(state.Client.State));
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
        _output->println(ClientStateName(after.State));
    }

    void OnAccessPointStateChanged(
        const ESPressio::WiFi::AccessPointRuntimeState& before,
        const ESPressio::WiFi::AccessPointRuntimeState& after) override {
        if (_output == nullptr) return;
        Prefix();
        _output->print("AP ");
        _output->print(APStateName(before.State));
        _output->print(" -> ");
        _output->println(APStateName(after.State));
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
    }

    void OnClientIPAddressLost() override {
        if (_output == nullptr) return;
        Prefix();
        _output->println("ClientIPAddressLost");
    }

    void OnClientNoKnownNetworkAvailable() override {
        if (_output == nullptr) return;
        Prefix();
        _output->println("NoKnownNetworkAvailable");
    }
};

} // namespace ESPressio::Serial
