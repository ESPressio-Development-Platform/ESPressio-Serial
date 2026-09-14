#pragma once

#if !__has_include(<ESPressio_SocketAdapterTransport.hpp>)
#error "SocketAdapterTransportMonitor requires the final ESPressio Sockets A2 transport."
#endif

#include <Arduino.h>
#include <cinttypes>
#include <cstdio>
#include <ESPressio_SocketAdapterTransport.hpp>

namespace ESPressio::Serial {

/// <summary>Prints bounded lifecycle diagnostics for a caller-owned final SocketAdapterTransport.</summary>
/// <remarks>The monitor owns no socket worker, session registry, queue, retry engine, or Primitive-family state.</remarks>
class SocketAdapterTransportMonitor final {
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

    template<class TTransport>
    bool PrintStatus(const TTransport& transport) {
        if (_output == nullptr) return false;
        _output->print("[ESPressio Sockets] [A2] active=");
        _output->print(transport.IsActive() ? "true" : "false");
        _output->print(" valid=");
        _output->print(transport.Validate() ? "true" : "false");
        _output->print(" generation=");
        PrintUnsigned(transport.Generation());
        _output->println();
        return true;
    }
};

} // namespace ESPressio::Serial
