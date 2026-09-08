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
/**
 * ESPressio Memory Audit
 * Members:
 * - SystemClock (bool): 1 bytes [0 bytes dynamic allocation]
 * - Threads (bool): 1 bytes [0 bytes dynamic allocation]
 * - Events (bool): 1 bytes [0 bytes dynamic allocation]
 * - Commands (bool): 1 bytes [0 bytes dynamic allocation]
 * - ESPNow (bool): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: 5 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
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
/**
 * ESPressio Memory Audit
 * Members:
 * - _systemClock (SystemClockMonitor<>): 20 bytes [_handle: owned object: 4 bytes]
 * - _threads (ThreadMonitor): 36 bytes [_managerHandle: owned object: 4 bytes; _terminationHandle: owned object: 4 bytes]
 * - _events (EventMonitor): 232 bytes [_observerHandle: owned object: 4 bytes; _worker: _handler: Name: Capacity + 1 bytes when capacity exceeds 15-byte SSO; _worker: _handler: LastId: Capacity + 1 bytes when capacity exceeds 15-byte SSO; _worker: _queue: owned object: 4 bytes; _worker: _startGate: owned object: 4 bytes; _worker: _lifecycleMutex: _owned: owned object: 4 bytes; _worker: _lifecycleMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; _mutex: native synchronization state may allocate platform resources lazily]
 * - _commands (CommandMonitor): 20 bytes [_handle: owned object: 4 bytes]
 * - _espNow (ESPNowTransportMonitor): 20 bytes known/aligned storage + sizeof(ESPressio::ESPNow::IESPNowTransportObserver) (target/toolchain dependent) [_handle: owned object: 4 bytes]
 * Total Memory: 308 bytes known/aligned storage + 20 bytes known/aligned storage + sizeof(ESPressio::ESPNow::IESPNowTransportObserver) (target/toolchain dependent) [_systemClock: _handle: owned object: 4 bytes; _threads: _managerHandle: owned object: 4 bytes; _threads: _terminationHandle: owned object: 4 bytes; _events: _observerHandle: owned object: 4 bytes; _events: _worker: _handler: Name: Capacity + 1 bytes when capacity exceeds 15-byte SSO; _events: _worker: _handler: LastId: Capacity + 1 bytes when capacity exceeds 15-byte SSO; _events: _worker: _queue: owned object: 4 bytes; _events: _worker: _startGate: owned object: 4 bytes; _events: _worker: _lifecycleMutex: _owned: owned object: 4 bytes; _events: _worker: _lifecycleMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; _events: _mutex: native synchronization state may allocate platform resources lazily; _commands: _handle: owned object: 4 bytes; _espNow: _handle: owned object: 4 bytes]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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
