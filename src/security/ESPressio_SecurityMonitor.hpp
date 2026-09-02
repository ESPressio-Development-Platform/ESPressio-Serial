#pragma once

#if !__has_include(<ESPressio_Security.hpp>)
#error "SecurityMonitor requires ESPressio Security >= 0.2.0 < 1.0.0."
#endif

#include <Arduino.h>
#include <ESPressio_Security.hpp>
#include <ESPressio_ITransportSecurityObserver.hpp>

namespace ESPressio::Serial {

/// <summary>Writes transport-security configuration, session, replay, and failure activity to an Arduino Print sink.</summary>
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
    /// <summary>Registers the monitor with a TransportSecurity instance and selects its output sink.</summary>
    /// <param name="output">Destination for diagnostic lines.</param>
    /// <param name="security">Transport security instance to observe.</param>
    /// <returns>True when the observer registration is active.</returns>
    bool Initialize(Print& output, ESPressio::Security::TransportSecurity& security) {
        if (_handle) return true;
        _output = &output;
        _handle = security.RegisterObserver(this);
        if (!_handle) { _output = nullptr; return false; }
        return true;
    }

    /// <summary>Unregisters from transport security and releases the output sink reference.</summary>
    void Shutdown() {
        _handle.reset();
        _output = nullptr;
    }

    /// <inheritdoc/>
    void OnTransportSecurityConfigurationChanged(
        const ESPressio::Security::TransportSecurityConfig&,
        const ESPressio::Security::TransportSecurityConfig&
    ) override { Line("ConfigurationChanged"); }

    /// <inheritdoc/>
    void OnTransportSecuritySessionReset(uint64_t previousSessionID) override {
        if (!_output) return;
        _output->print("[ESPressio Security] SessionReset previous=");
        _output->println(static_cast<unsigned long long>(previousSessionID));
    }

    /// <inheritdoc/>
    void OnTransportSecuritySessionEstablished(uint64_t sessionID) override {
        if (!_output) return;
        _output->print("[ESPressio Security] SessionEstablished id=");
        _output->println(static_cast<unsigned long long>(sessionID));
    }

    /// <inheritdoc/>
    void OnTransportSecurityReplayProtectionReset() override {
        Line("ReplayProtectionReset");
    }

    /// <inheritdoc/>
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
