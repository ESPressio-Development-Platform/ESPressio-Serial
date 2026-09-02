#pragma once

#if !__has_include(<ESPressio_ESPNowTransport.hpp>)
#error "ESPNowTransportMonitor requires ESPressio ESP-Now >= 0.5.0 < 1.0.0."
#endif

#include <Arduino.h>
#include <ESPressio_ESPNowTransport.hpp>
#include <ESPressio_IESPNowTransportObserver.hpp>

namespace ESPressio::Serial {

/// <summary>Writes ESP-NOW transport lifecycle, peer, and send activity to an Arduino Print sink.</summary>
class ESPNowTransportMonitor final :
    public ESPressio::ESPNow::IESPNowTransportObserver {
private:
    Print* _output = nullptr;
    ESPressio::Observable::ObserverHandlePtr _handle;

    void Line(const char* operation) {
        if (!_output) return;
        _output->print("[ESPressio ESP-Now] ");
        _output->println(operation);
    }

public:
    /// <summary>Registers the monitor with an ESP-NOW transport and selects its output sink.</summary>
    /// <param name="output">Destination for diagnostic lines.</param>
    /// <param name="transport">Transport to observe; defaults to the process-wide singleton.</param>
    /// <returns>True when the observer registration is active.</returns>
    bool Initialize(
        Print& output,
        ESPressio::ESPNow::ESPNowTransport& transport = ESPressio::ESPNow::ESPNowTransport::GetInstance()
    ) {
        if (_handle) return true;
        _output = &output;
        _handle = transport.RegisterObserver(this);
        if (!_handle) { _output = nullptr; return false; }
        return true;
    }

    /// <summary>Unregisters from the transport and releases the output sink reference.</summary>
    void Shutdown() {
        _handle.reset();
        _output = nullptr;
    }

    /// <inheritdoc/>
    void OnESPNowTransportInitialized() override { Line("Initialized"); }
    /// <inheritdoc/>
    void OnESPNowTransportInitializationFailed() override { Line("InitializationFailed"); }
    /// <inheritdoc/>
    void OnESPNowTransportShutdown() override { Line("Shutdown"); }
    /// <inheritdoc/>
    void OnESPNowPeerAdded(const ESPressio::ESPNow::MacAddress&) override { Line("PeerAdded"); }
    /// <inheritdoc/>
    void OnESPNowPeerRemoved(const ESPressio::ESPNow::MacAddress&) override { Line("PeerRemoved"); }
    /// <inheritdoc/>
    void OnESPNowSendAccepted(const ESPressio::ESPNow::MacAddress&, uint8_t, std::size_t) override { Line("SendAccepted"); }
    /// <inheritdoc/>
    void OnESPNowSendFailed(const ESPressio::ESPNow::MacAddress&, uint8_t, std::size_t) override { Line("SendFailed"); }
};

} // namespace ESPressio::Serial
