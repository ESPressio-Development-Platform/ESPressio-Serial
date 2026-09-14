#pragma once

#include <Arduino.h>

#if __has_include(<ESPressio_Timing.hpp>)
#include "../timing/ESPressio_SystemClockMonitor.hpp"
#define ESPRESSIO_SERIAL_HAS_TIMING_MONITOR 1
#endif

#if __has_include(<ESPressio_IThread.hpp>)
#include "../threads/ESPressio_ThreadMonitor.hpp"
#define ESPRESSIO_SERIAL_HAS_THREAD_MONITOR 1
#endif

#if defined(ARDUINO_ARCH_ESP32) && __has_include(<ESPressio_ESPNowRadio.hpp>)
#include "../espnow/ESPressio_ESPNowRadioMonitor.hpp"
#define ESPRESSIO_SERIAL_HAS_ESPNOW_MONITOR 1
#endif

namespace ESPressio::Serial {

/// Selects the final provider diagnostics aggregated by DiagnosticMonitor.
/// Primitive-family discovery/inspection remains in the dedicated Command/Event/State tooling,
/// whose platform-neutral output seam is intentionally independent of this Arduino renderer.
struct DiagnosticMonitorConfig final {
    bool SystemClock = true;
    bool Threads = true;
    bool ESPNow = false;
};

/// Aggregates final provider diagnostic renderers without acquiring provider ownership.
///
/// Timing observation attaches only to the caller-selected SystemClock. Thread and ESP-NOW
/// diagnostics remain point-in-time reads of caller-owned instances. No Primitive TypeDirectory,
/// family registry, transport, retry path, scheduler, worker, event-listener topology, or provider
/// lifecycle is owned here.
class DiagnosticMonitor final {
private:
    bool _initialized = false;

#ifdef ESPRESSIO_SERIAL_HAS_TIMING_MONITOR
    SystemClockMonitor<> _systemClock;
#endif
#ifdef ESPRESSIO_SERIAL_HAS_THREAD_MONITOR
    ThreadMonitor _threads;
#endif
#ifdef ESPRESSIO_SERIAL_HAS_ESPNOW_MONITOR
    ESPNowRadioMonitor _espNow;
#endif

public:
    DiagnosticMonitor() = default;
    DiagnosticMonitor(const DiagnosticMonitor&) = delete;
    DiagnosticMonitor& operator=(const DiagnosticMonitor&) = delete;
    ~DiagnosticMonitor() { Shutdown(); }

    /// Initializes only selected final diagnostic renderers.
    bool Initialize(::Print& output, const DiagnosticMonitorConfig& config = {}) {
        Shutdown();
        bool success = true;
#ifdef ESPRESSIO_SERIAL_HAS_TIMING_MONITOR
        if (config.SystemClock) success = _systemClock.Initialize(output) && success;
#else
        if (config.SystemClock) success = false;
#endif
#ifdef ESPRESSIO_SERIAL_HAS_THREAD_MONITOR
        if (config.Threads) success = _threads.Initialize(output) && success;
#else
        if (config.Threads) success = false;
#endif
#ifdef ESPRESSIO_SERIAL_HAS_ESPNOW_MONITOR
        if (config.ESPNow) success = _espNow.Initialize(output) && success;
#else
        if (config.ESPNow) success = false;
#endif
        _initialized = success;
        if (!success) Shutdown();
        return success;
    }

    void Shutdown() noexcept {
#ifdef ESPRESSIO_SERIAL_HAS_ESPNOW_MONITOR
        _espNow.Shutdown();
#endif
#ifdef ESPRESSIO_SERIAL_HAS_THREAD_MONITOR
        _threads.Shutdown();
#endif
#ifdef ESPRESSIO_SERIAL_HAS_TIMING_MONITOR
        _systemClock.Shutdown();
#endif
        _initialized = false;
    }

    bool IsInitialized() const noexcept { return _initialized; }

#ifdef ESPRESSIO_SERIAL_HAS_THREAD_MONITOR
    bool PrintThreadStatus(const Threads::IThread& thread, const char* name = nullptr) {
        return _initialized && _threads.PrintStatus(thread, name);
    }
#endif

#ifdef ESPRESSIO_SERIAL_HAS_ESPNOW_MONITOR
    bool PrintESPNowStatus(const ESPNow::ESPNowRadio& radio) {
        return _initialized && _espNow.PrintStatus(radio);
    }
#endif
};

} // namespace ESPressio::Serial
