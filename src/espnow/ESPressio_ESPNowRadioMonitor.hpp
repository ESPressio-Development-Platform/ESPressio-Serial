#pragma once

#if !defined(ARDUINO_ARCH_ESP32)
#error "ESPNowRadioMonitor requires an ESP32 Arduino target."
#endif
#if !__has_include(<ESPressio_ESPNowRadio.hpp>)
#error "ESPNowRadioMonitor requires the final ESPressio ESP-Now Radio provider."
#endif

#include <Arduino.h>
#include <cinttypes>
#include <cstdio>
#include <ESPressio_ESPNowRadio.hpp>

namespace ESPressio::Serial {

/// <summary>Prints bounded point-in-time diagnostics from the final ESP-NOW physical Radio provider.</summary>
/// <remarks>Owns no transport observer, protocol registry, worker, retry path, or Primitive-family state.</remarks>
class ESPNowRadioMonitor final {
private:
    Print* _output = nullptr;

    void PrintUnsigned(std::uint64_t value) {
        char buffer[32]{};
        std::snprintf(buffer, sizeof(buffer), "%" PRIu64, value);
        _output->print(buffer);
    }

public:
    bool Initialize(Print& output) noexcept {
        _output = &output;
        return true;
    }

    void Shutdown() noexcept { _output = nullptr; }
    bool IsInitialized() const noexcept { return _output != nullptr; }

    bool PrintStatus(const ESPressio::ESPNow::ESPNowRadio& radio) {
        if (_output == nullptr) return false;
        _output->print("[ESPressio ESP-Now Radio] started=");
        _output->print(radio.IsStarted() ? "true" : "false");
        _output->print(" transmit-ready=");
        _output->print(radio.IsTransmitReady() ? "true" : "false");
        _output->print(" accepted-rx=");
        PrintUnsigned(radio.AcceptedIngressPackets());
        _output->print(" dropped-rx=");
        PrintUnsigned(radio.DroppedIngressPackets());
        _output->print(" rx-high-watermark=");
        _output->println(static_cast<unsigned long>(radio.IngressHighWatermark()));
        return true;
    }
};

} // namespace ESPressio::Serial
