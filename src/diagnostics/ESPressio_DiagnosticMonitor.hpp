#pragma once

#include <Arduino.h>
#include <string_view>

#if __has_include(<ESPressio_Timing.hpp>)
#include "../timing/ESPressio_SystemClockMonitor.hpp"
#define ESPRESSIO_SERIAL_HAS_TIMING_MONITOR 1
#endif

#if __has_include(<ESPressio_IThread.hpp>)
#include "../threads/ESPressio_ThreadMonitor.hpp"
#define ESPRESSIO_SERIAL_HAS_THREAD_MONITOR 1
#endif

#if __has_include(<ESPressio_TypeDirectory.hpp>)
#include <ESPressio_TypeDirectory.hpp>
#define ESPRESSIO_SERIAL_HAS_PRIMITIVE_DIRECTORY 1
#endif

#if defined(ESPRESSIO_SERIAL_HAS_PRIMITIVE_DIRECTORY) && __has_include(<ESPressio_EventTypeDescriptor.hpp>)
#include "../event/ESPressio_EventMonitor.hpp"
#define ESPRESSIO_SERIAL_HAS_EVENT_MONITOR 1
#endif

#if defined(ESPRESSIO_SERIAL_HAS_PRIMITIVE_DIRECTORY) && __has_include(<ESPressio_CommandDescriptor.hpp>)
#include "../command/ESPressio_CommandMonitor.hpp"
#define ESPRESSIO_SERIAL_HAS_COMMAND_MONITOR 1
#endif

#if defined(ARDUINO_ARCH_ESP32) && __has_include(<ESPressio_ESPNowRadio.hpp>)
#include "../espnow/ESPressio_ESPNowRadioMonitor.hpp"
#define ESPRESSIO_SERIAL_HAS_ESPNOW_MONITOR 1
#endif

namespace ESPressio::Serial {

/// Selects the final diagnostic seams aggregated by DiagnosticMonitor.
/// Event/Command descriptor diagnostics are disabled by default because they
/// require the application to provide its frozen Primitive TypeDirectory.
struct DiagnosticMonitorConfig final {
    bool SystemClock = true;
    bool Threads = true;
    bool Events = false;
    bool Commands = false;
    bool ESPNow = false;
};

/// Aggregates final ESPressio diagnostic renderers without acquiring provider ownership.
///
/// Timing observation attaches only to the caller-selected SystemClock. Thread and ESP-NOW
/// diagnostics remain point-in-time reads of caller-owned instances. Event/Command discovery
/// borrows one frozen Primitive TypeDirectory. No registry, transport, retry path, scheduler,
/// worker, event listener topology, or provider lifecycle is owned here.
class DiagnosticMonitor final {
private:
    bool _initialized = false;

#ifdef ESPRESSIO_SERIAL_HAS_TIMING_MONITOR
    SystemClockMonitor<> _systemClock;
#endif
#ifdef ESPRESSIO_SERIAL_HAS_THREAD_MONITOR
    ThreadMonitor _threads;
#endif
#ifdef ESPRESSIO_SERIAL_HAS_EVENT_MONITOR
    EventMonitor _events;
#endif
#ifdef ESPRESSIO_SERIAL_HAS_COMMAND_MONITOR
    CommandMonitor _commands;
#endif
#ifdef ESPRESSIO_SERIAL_HAS_ESPNOW_MONITOR
    ESPNowRadioMonitor _espNow;
#endif

    bool InitializeNonFamily(Print& output, const DiagnosticMonitorConfig& config) {
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
        return success;
    }

public:
    DiagnosticMonitor() = default;
    DiagnosticMonitor(const DiagnosticMonitor&) = delete;
    DiagnosticMonitor& operator=(const DiagnosticMonitor&) = delete;
    ~DiagnosticMonitor() { Shutdown(); }

    /// Initializes diagnostics that do not require Primitive TypeDirectory discovery.
    /// Requesting Event/Command diagnostics through this overload fails closed.
    bool Initialize(Print& output, const DiagnosticMonitorConfig& config = {}) {
        Shutdown();
        bool success = InitializeNonFamily(output, config);
        if (config.Events || config.Commands) success = false;
        _initialized = success;
        if (!success) Shutdown();
        return success;
    }

#ifdef ESPRESSIO_SERIAL_HAS_PRIMITIVE_DIRECTORY
    /// Initializes final diagnostics and borrows the application's frozen Primitive TypeDirectory.
    bool Initialize(
        Print& output,
        Primitive::TypeDirectoryView types,
        const DiagnosticMonitorConfig& config = {}
    ) {
        Shutdown();
        if ((config.Events || config.Commands) && !types.IsFrozen()) return false;

        bool success = InitializeNonFamily(output, config);
#ifdef ESPRESSIO_SERIAL_HAS_EVENT_MONITOR
        if (config.Events) success = _events.Initialize(types, output) && success;
#else
        if (config.Events) success = false;
#endif
#ifdef ESPRESSIO_SERIAL_HAS_COMMAND_MONITOR
        if (config.Commands) success = _commands.Initialize(types, output) && success;
#else
        if (config.Commands) success = false;
#endif
        _initialized = success;
        if (!success) Shutdown();
        return success;
    }
#endif

    void Shutdown() noexcept {
#ifdef ESPRESSIO_SERIAL_HAS_COMMAND_MONITOR
        _commands.Shutdown();
#endif
#ifdef ESPRESSIO_SERIAL_HAS_EVENT_MONITOR
        _events.Shutdown();
#endif
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

#ifdef ESPRESSIO_SERIAL_HAS_EVENT_MONITOR
    void ListEventTypes() const noexcept {
        if (_initialized) _events.List();
    }
    bool DescribeEventType(std::string_view canonicalName) const noexcept {
        return _initialized && _events.Describe(canonicalName);
    }
#endif

#ifdef ESPRESSIO_SERIAL_HAS_COMMAND_MONITOR
    void ListCommandTypes() const noexcept {
        if (_initialized) _commands.List();
    }
    bool DescribeCommandType(std::string_view canonicalName) const noexcept {
        return _initialized && _commands.Describe(canonicalName);
    }
#endif

#ifdef ESPRESSIO_SERIAL_HAS_ESPNOW_MONITOR
    bool PrintESPNowStatus(const ESPNow::ESPNowRadio& radio) {
        return _initialized && _espNow.PrintStatus(radio);
    }
#endif
};

} // namespace ESPressio::Serial
