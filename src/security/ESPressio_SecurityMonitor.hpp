#pragma once

#if !__has_include(<ESPressio_Security.hpp>)
#error "SecurityMonitor requires ESPressio Security >= 0.2.0 < 1.0.0."
#endif

#include <Arduino.h>
#include <ESPressio_Security.hpp>
#include <ESPressio_ITransportSecurityObserver.hpp>

namespace ESPressio::Serial {

class SecurityMonitor final :
    public ESPressio::Security::ITransportSecurityObserver {
private:
    Print* _output = nullptr;
    ESPressio::Observable::ObserverHandlePtr _handle;

    void Line(const char* operation) {
        if (!_output) return;
        _output->print("[ESPressio Security] ");
        _output->println(operation);
    }

public:
    bool Initialize(Print& output, ESPressio::Security::TransportSecurity& security) {
        if (_handle) return true;
        _output = &output;
        _handle = security.RegisterObserver(this);
        if (!_handle) { _output = nullptr; return false; }
        return true;
    }

    void Shutdown() {
        _handle.reset();
        _output = nullptr;
    }

    void OnTransportSecurityConfigurationChanged(
        const ESPressio::Security::TransportSecurityConfig&,
        const ESPressio::Security::TransportSecurityConfig&
    ) override { Line("ConfigurationChanged"); }

    void OnTransportSecuritySessionReset(uint64_t previousSessionID) override {
        if (!_output) return;
        _output->print("[ESPressio Security] SessionReset previous=");
        _output->println(static_cast<unsigned long long>(previousSessionID));
    }

    void OnTransportSecuritySessionEstablished(uint64_t sessionID) override {
        if (!_output) return;
        _output->print("[ESPressio Security] SessionEstablished id=");
        _output->println(static_cast<unsigned long long>(sessionID));
    }

    void OnTransportSecurityReplayProtectionReset() override {
        Line("ReplayProtectionReset");
    }

    void OnTransportSecurityFailure(const ESPressio::Security::SecurityResult& result) override {
        if (!_output) return;
        _output->print("[ESPressio Security] Failure error=");
        _output->print(static_cast<unsigned int>(result.Error));
        if (!result.Message.empty()) {
            _output->print(" message=");
            _output->print(result.Message.c_str());
        }
        _output->println();
    }
};

} // namespace ESPressio::Serial
