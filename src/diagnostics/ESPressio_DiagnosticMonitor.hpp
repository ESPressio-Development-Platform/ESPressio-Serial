#pragma once
#include <Arduino.h>

#if __has_include(<ESPressio_Timing.hpp>)
#include "../timing/ESPressio_SystemClockMonitor.hpp"
#define ESPRESSIO_SERIAL_HAS_TIMING_MONITOR 1
#endif

#if __has_include(<ESPressio_ThreadManager.hpp>)
#include "../threads/ESPressio_ThreadMonitor.hpp"
#define ESPRESSIO_SERIAL_HAS_THREAD_MONITOR 1
#endif

#if __has_include(<ESPressio_EventTransport.hpp>) && __has_include(<ESPressio_BinaryArchive.hpp>)
#include "../event/ESPressio_EventMonitor.hpp"
#define ESPRESSIO_SERIAL_HAS_EVENT_MONITOR 1
#endif

#if __has_include(<ESPressio_Command.hpp>) && __has_include(<ESPressio_ICommandRegistryObserver.hpp>)
#include "../command/ESPressio_CommandMonitor.hpp"
#define ESPRESSIO_SERIAL_HAS_COMMAND_MONITOR 1
#endif

#if __has_include(<ESPressio_ESPNowTransport.hpp>) && __has_include(<ESPressio_IESPNowTransportObserver.hpp>)
#include "../espnow/ESPressio_ESPNowTransportMonitor.hpp"
#define ESPRESSIO_SERIAL_HAS_ESPNOW_MONITOR 1
#endif

namespace ESPressio::Serial {

/// <summary>Selects which available subsystem monitors are aggregated by DiagnosticMonitor.</summary>
struct DiagnosticMonitorConfig {
    /// <summary>Enable System Clock diagnostics when Timing support is available.</summary>
    bool SystemClock = true;
    /// <summary>Enable thread diagnostics when Threads support is available.</summary>
    bool Threads = true;
    /// <summary>Enable Event diagnostics when Event transport support is available.</summary>
    bool Events = true;
    /// <summary>Enable Command diagnostics when Command observer support is available.</summary>
    bool Commands = false;
    /// <summary>Enable ESP-NOW diagnostics when ESP-NOW observer support is available.</summary>
    bool ESPNow = false;
};

/// <summary>Aggregates the optional subsystem monitors compiled into ESPressio-Serial behind one lifecycle.</summary>
/// <remarks>A requested subsystem that is not available at compile time causes Initialize to report failure while still initializing the other requested monitors.</remarks>
class DiagnosticMonitor final {
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
    ESPNowTransportMonitor _espNow;
#endif
public:
    /// <summary>Initializes the selected diagnostic monitors against a shared Arduino Print sink.</summary>
    /// <param name="output">Destination used by all enabled monitors.</param>
    /// <param name="config">Subsystems to enable.</param>
    /// <returns>True only when every requested monitor is available and initializes successfully.</returns>
    bool Initialize(Print& output, const DiagnosticMonitorConfig& config = {}) {
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
#ifdef ESPRESSIO_SERIAL_HAS_EVENT_MONITOR
        if (config.Events) success = _events.Initialize(output) && success;
#else
        if (config.Events) success = false;
#endif
#ifdef ESPRESSIO_SERIAL_HAS_COMMAND_MONITOR
        if (config.Commands) success = _commands.Initialize(output) && success;
#else
        if (config.Commands) success = false;
#endif
#ifdef ESPRESSIO_SERIAL_HAS_ESPNOW_MONITOR
        if (config.ESPNow) success = _espNow.Initialize(output) && success;
#else
        if (config.ESPNow) success = false;
#endif
        return success;
    }

    /// <summary>Shuts down every compiled monitor in reverse initialization dependency order.</summary>
    void Shutdown() {
#ifdef ESPRESSIO_SERIAL_HAS_ESPNOW_MONITOR
        _espNow.Shutdown();
#endif
#ifdef ESPRESSIO_SERIAL_HAS_COMMAND_MONITOR
        _commands.Shutdown();
#endif
#ifdef ESPRESSIO_SERIAL_HAS_EVENT_MONITOR
        _events.Shutdown();
#endif
#ifdef ESPRESSIO_SERIAL_HAS_THREAD_MONITOR
        _threads.Shutdown();
#endif
#ifdef ESPRESSIO_SERIAL_HAS_TIMING_MONITOR
        _systemClock.Shutdown();
#endif
    }
};
}
