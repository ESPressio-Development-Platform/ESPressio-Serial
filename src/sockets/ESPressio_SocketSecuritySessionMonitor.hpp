#pragma once

#if !__has_include(<ESPressio_SocketSecuritySession.hpp>)
#error "SocketSecuritySessionMonitor requires ESPressio Sockets >= 0.5.0 < 1.0.0 and ESPressio Security >= 0.2.0 < 1.0.0."
#endif

#include <Arduino.h>
#include <ESPressio_ISocketSecuritySessionObserver.hpp>
#include <ESPressio_SocketSecuritySession.hpp>

namespace ESPressio::Serial {

/// <summary>Writes Socket security-session fault and reset activity to an Arduino Print sink.</summary>
class SocketSecuritySessionMonitor final :
    public ESPressio::Sockets::ISocketSecuritySessionObserver {
private:
    Print* _output = nullptr;
    ESPressio::Observable::ObserverHandlePtr _handle;

public:
    /// <summary>Registers the monitor with a SocketSecuritySession and selects its output sink.</summary>
    /// <returns>True when the observer registration is active.</returns>
    bool Initialize(Print& output, ESPressio::Sockets::SocketSecuritySession& session) {
        if (_handle) return true;
        _output = &output;
        _handle = session.RegisterObserver(this);
        if (!_handle) { _output = nullptr; return false; }
        return true;
    }

    /// <summary>Unregisters from the security session and releases the output sink reference.</summary>
    void Shutdown() {
        _handle.reset();
        _output = nullptr;
    }

    /// <inheritdoc/>
    void OnSocketSecuritySessionFaulted(const ESPressio::Security::SecurityResult& result) override {
        if (!_output) return;
        _output->print("[ESPressio Sockets] [SecuritySession] Faulted error=");
        _output->print(static_cast<unsigned int>(result.Error));
        if (!result.Message.empty()) {
            _output->print(" message=");
            _output->print(result.Message.c_str());
        }
        _output->println();
    }

    /// <inheritdoc/>
    void OnSocketSecuritySessionReset() override {
        if (!_output) return;
        _output->println("[ESPressio Sockets] [SecuritySession] Reset");
    }
};

} // namespace ESPressio::Serial
