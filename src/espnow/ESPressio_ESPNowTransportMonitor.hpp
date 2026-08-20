#pragma once

#if !__has_include(<ESPressio_ESPNowTransport.hpp>)
#error "ESPNowTransportMonitor requires ESPressio ESP-Now >= 0.5.0 < 1.0.0."
#endif

#include <Arduino.h>
#include <ESPressio_ESPNowTransport.hpp>
#include <ESPressio_IESPNowTransportObserver.hpp>

namespace ESPressio::Serial {

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

    void Shutdown() {
        _handle.reset();
        _output = nullptr;
    }

    void OnESPNowTransportInitialized() override { Line("Initialized"); }
    void OnESPNowTransportInitializationFailed() override { Line("InitializationFailed"); }
    void OnESPNowTransportShutdown() override { Line("Shutdown"); }
    void OnESPNowPeerAdded(const ESPressio::ESPNow::MacAddress&) override { Line("PeerAdded"); }
    void OnESPNowPeerRemoved(const ESPressio::ESPNow::MacAddress&) override { Line("PeerRemoved"); }
    void OnESPNowSendAccepted(const ESPressio::ESPNow::MacAddress&, uint8_t, std::size_t) override { Line("SendAccepted"); }
    void OnESPNowSendFailed(const ESPressio::ESPNow::MacAddress&, uint8_t, std::size_t) override { Line("SendFailed"); }
};

} // namespace ESPressio::Serial
