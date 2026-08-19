# ESPressio Serial

Serial and console-oriented components for the Flowduino ESPressio Development Platform.

Version 0.2.0 expands ESPressio Serial into the human-facing diagnostics and logging layer of the ESPressio ecosystem, with structured logging, pluggable sinks, retained diagnostic history, and opt-in monitors for Event, Timing, and Threads.

## Latest Stable Version

The latest stable version is **0.2.0**.

## ESPressio Development Platform

ESPressio is a collection of discrete, composable component libraries designed around a common development ethos:

- **Light-weight**
- **Ease of use**
- **Object-oriented design**
- **SOLID design principles**
- **Pay only for the functionality an application selects**

## License

Licensed under the **Apache License 2.0**. See [LICENSE](LICENSE).

## Namespace

The public API resides beneath:

```cpp
ESPressio::Serial
```

Because Arduino exposes a global object named `Serial`, fully qualified ESPressio Serial names are recommended:

```cpp
ESPressio::Serial::EventMonitor monitor;
```

while the Arduino serial port remains:

```cpp
::Serial
```

## ESPressio Library Dependencies

The **core ESPressio Serial library has no required ESPressio library dependencies**.

The Event Monitor is deliberately opt-in and requires:

```text
ESPressio Event >= 5.5.0
ESPressio Serializable >= 0.9.0
```

For the complete ecosystem hierarchy, see:

**[ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.md)**

In the dependency chart:

- **Solid relationships** represent required dependencies.
- **Dashed relationships** represent opt-in dependencies introduced only when the associated feature/header is used.

---


# Version 0.2.0 — Diagnostics & Logging

Version 0.2.0 adds a general diagnostics foundation alongside the existing Event Monitor.

## Logging

```cpp
#include <ESPressio_Logging.hpp>

ESPressio::Serial::Logger<> logger;
ESPressio::Serial::SerialLogSink serialSink(::Serial);
ESPressio::Serial::DiagnosticRingBuffer<64> history;

void setup() {
    ::Serial.begin(115200);

    logger.AddSink(serialSink);
    logger.AddSink(history);

    logger.SetMinimumLevel(
        ESPressio::Serial::LogLevel::Debug
    );

    logger.Info("Application", "Boot complete");
}
```

Supported levels are:

```text
Trace
Debug
Info
Warning
Error
Critical
Off
```

`Logger` supports multiple simultaneous `ILoggerSink` implementations. Logging data is therefore separated from its output destination: Serial is one sink, not the logging architecture itself.

`ESPRESSIO_SERIAL_COMPILETIME_LOG_LEVEL` may be defined to remove lower-severity calls from runtime delivery, while `SetMinimumLevel()` provides runtime filtering.

## Diagnostic flight recorder

`DiagnosticRingBuffer<Capacity>` is both an `ILoggerSink` and a bounded in-memory history.

```cpp
ESPressio::Serial::DiagnosticRingBuffer<64> history;

logger.AddSink(history);

// Later, after a fault:
history.Dump(::Serial);
```

Entries are copied into fixed-size storage; the oldest entry is overwritten when capacity is exhausted. This makes it suitable for retaining the diagnostic events immediately preceding a failure without unbounded heap growth.

## System Clock Monitor

```cpp
#include <ESPressio_SystemClockMonitor.hpp>

ESPressio::Serial::SystemClockMonitor<> clockMonitor;

clockMonitor.Initialize(::Serial);
```

This integration directly consumes ESPressio Timing 2.2.0's `ISystemClockObserver` notifications. It reports time-setting, synchronization acceptance/rejection, synchronization state changes, resets/configuration changes, and callback scheduling/execution.

Synchronization output includes the clock value before correction, the value after correction, and the immediate nanosecond difference.

This is an opt-in Timing dependency; ESPressio Event is not involved.

## Thread Monitor

```cpp
#include <ESPressio_ThreadMonitor.hpp>

ESPressio::Serial::ThreadMonitor threadMonitor;

threadMonitor.Initialize(::Serial);
```

`ThreadMonitor` directly observes the process-wide ESPressio Threads 3.1.0 infrastructure:

```text
ThreadManager
ThreadGarbageCollector
ThreadTerminationDispatcher
```

It reports registration, cleanup, garbage collection, termination dispatch, initialization, and failure lifecycle notifications.

This is an opt-in Threads dependency; Event bridges are not required merely to display Thread diagnostics.

## Aggregate Diagnostic Monitor

When the corresponding dependency headers are available, the convenience monitor can compose all supported subsystem monitors:

```cpp
#include <ESPressio_DiagnosticMonitor.hpp>

ESPressio::Serial::DiagnosticMonitor diagnostics;

void setup() {
    ::Serial.begin(115200);

    ESPressio::Serial::DiagnosticMonitorConfig config;

    config.SystemClock = true;
    config.Threads = true;
    config.Events = true;

    diagnostics.Initialize(
        ::Serial,
        config
    );
}
```

The aggregate uses compile-time feature detection. It does not itself make Timing, Threads, Event, or Serializable mandatory package dependencies.

## Dependency model

```text
ESPressio Serial core
    -> no mandatory ESPressio dependency

Logging
    -> no additional ESPressio dependency

SystemClockMonitor
    - - -> ESPressio Timing >= 2.2.0

ThreadMonitor
    - - -> ESPressio Threads >= 3.1.0

EventMonitor
    - - -> ESPressio Event >= 5.5.0
    - - -> ESPressio Serializable >= 0.9.0
```

All ESPressio relationships remain opt-in.

---

# Core include

```cpp
#include <ESPressio_Serial.hpp>
```

The core header contains common ESPressio Serial types only.

It does **not** include ESPressio Event or ESPressio Serializable.

Event monitoring is selected explicitly:

```cpp
#include <ESPressio_EventMonitor.hpp>
```

or through the feature batch header:

```cpp
#include <ESPressio_SerialEventMonitoring.hpp>
```

---

# Event Transport Monitor

`EventMonitor` consumes the **Event Transport Transaction Observation** API introduced by ESPressio Event 5.5.0.

It does not implement an Event Transport and does not alter Event routing.

Conceptually:

```text
Serializable Event
        |
        v
EventTransportManager
        |
        +-----------------------> concrete transport
        |
        +--> transaction Observer
                    |
                    v
              EventMonitor
                    |
                    v
              Arduino Print
                    |
           +--------+--------+
           |                 |
           v                 v
        Serial            USB CDC
```

Any `Print` implementation may be used. The monitor is therefore not tied specifically to `HardwareSerial`.

## Initialization

```cpp
#include <ESPressio_EventMonitor.hpp>

ESPressio::Serial::EventMonitor
    monitor;

void setup() {
    ::Serial.begin(115200);

    ESPressio::Serial::
        EventMonitorConfig
            config;

    monitor.Initialize(
        ::Serial,
        config
    );
}
```

`Initialize()` registers the monitor with the selected `EventTransportManager`.

It does **not** initialize the Event Transport Manager itself. The application remains responsible for its normal Event Transport setup and initialization.

The monitor unregisters automatically when destroyed or when:

```cpp
monitor.Shutdown();
```

is called.

---

# Default monitoring behaviour

The default mode is:

```cpp
EventMonitorMode::Events
```

This is intended to provide one useful record per logical transported Event rather than printing every internal lifecycle transition.

It reports:

```text
outbound Event after concrete transport handoff
inbound Event after successful deserialization
inbound rejection
transport processing failure
```

For example:

```text
[ESPressio Event] [OUT] [OutboundHandedToTransport] type=flowduino.example.serial.monitored-counter.v1 typeId=0x... schema=1 message=3 transport=0x... dispatch=Queue priority=Normal origin=Local hops=0 accepted=true payloadBytes=...
  payload: {
    "__schemaVersion": 1,
    "counter": 3,
    "source": "local"
  }
```

and the looped-back inbound Event may then appear as:

```text
[ESPressio Event] [IN] [InboundDeserialized] type=flowduino.example.serial.monitored-counter.v1 typeId=0x... schema=1 message=3 transport=0x... dispatch=Queue priority=Normal origin=Remote hops=0 payloadBytes=...
  payload: {
    "__schemaVersion": 1,
    "counter": 3,
    "source": "local"
  }
```

---

# Lifecycle mode

For deeper diagnostics:

```cpp
config.Mode =
    ESPressio::Serial::
        EventMonitorMode::Lifecycle;
```

prints every Event 5.5 transaction stage exposed by `EventTransportManager`:

```text
OutboundAccepted
OutboundSerialized
OutboundHandedToTransport

InboundAccepted
InboundRejected
InboundDeserialized
InboundDispatched

Failed
```

Lifecycle mode is intentionally verbose and is most useful while debugging the transport pipeline itself.

---

# Payload formatting

The Event Monitor supports:

```cpp
EventMonitorPayloadFormat::None
EventMonitorPayloadFormat::Summary
EventMonitorPayloadFormat::Hex
EventMonitorPayloadFormat::Structured
```

## `None`

Only transaction metadata is printed.

## `Summary`

Reports payload size without printing payload contents.

## `Hex`

Prints the Serializable Binary Archive bytes in hexadecimal.

The maximum number of bytes is controlled by:

```cpp
config.MaximumHexPayloadBytes
```

## `Structured`

`Structured` is the default.

ESPressio Event Transport serializes Event payloads using ESPressio Serializable's `BinaryArchive`.

The monitor decodes that Binary Archive into Serializable's generic `SerializationNode` tree and renders it directly as JSON-like structured text.

This has two important advantages:

1. the monitor does not need to know the concrete C++ Event type;
2. it does not require ArduinoJson merely to present human-readable diagnostics.

The monitor is therefore able to inspect arbitrary transported Serializable Event payloads using the schema already encoded in the Binary Archive.

---

# Structured-output limits

Diagnostic output should not be allowed to grow without bound.

Configuration includes:

```cpp
MaximumCollectionItems
MaximumStringLength
MaximumStructuredDepth
IndentSpaces
PrettyStructuredPayload
```

These provide deterministic limits when monitoring large or deeply nested Event payloads.

---

# Transaction metadata

The monitor can independently enable or disable:

```text
stable Event type name
stable Event type ID
schema version
message ID
transport address
dispatch method
priority
origin
hop count
transport acceptance result
```

using `EventMonitorConfig`.

Inbound and outbound monitoring can also be enabled independently.

---

# Borrowed Event Transport data

ESPressio Event 5.5 transaction snapshots expose borrowed Event/payload references valid only during the Observer callback.

`EventMonitor` consumes those values synchronously and does not retain borrowed transaction pointers after the callback returns.

Structured decoding is therefore performed while the payload is valid.

---

# Performance considerations

Event Transport transaction observation is synchronous.

Serial/USB output can be comparatively slow.

Enabling Event Monitor—particularly `Lifecycle` mode or large structured payload output—can therefore add diagnostic latency to the Event Transport execution path.

This is intentional for a developer-facing monitor, but applications with tight real-time requirements should:

- disable monitoring in production;
- use `Summary` or `None` payload modes;
- use an appropriately fast `Print` destination;
- avoid Lifecycle mode except during diagnosis.

The Event Monitor changes observation only; it does not change Event routing or transport semantics.

---

# Example

The repository includes:

```text
examples/
└── EventMonitor/
    └── EventMonitor.ino
```

The example uses a small local `LoopbackEventTransport` so both outbound and inbound transactions can be demonstrated on a single ESP32 without networking or additional hardware.

It defines a Serializable counter Event, transports it through Event 5.5, and renders the Binary payload as structured text.

---

# PlatformIO

A project using only the core Serial library:

```ini
lib_deps =
    flowduino/ESPressio-Serial@^0.2.0
```

An application using Event Monitor requires:

```ini
lib_deps =
    flowduino/ESPressio-Serial@^0.2.0
    flowduino/ESPressio-Event@^5.5.0
    flowduino/ESPressio-Serializable@^0.9.0
```

The Event/Serializable dependencies are intentionally not declared as mandatory package dependencies of ESPressio Serial because they are required only by the opt-in Event Monitor feature.

---

# Future direction

ESPressio Serial is intended to contain Serial/console-oriented ESPressio integrations rather than becoming a general communications catch-all.

Potential future components include:

```text
Serial Event Transport
Serial command/console infrastructure
System Clock / Threads diagnostic monitors
structured ESPressio diagnostics output
serial configuration interfaces
serial protocol adapters
```

Network/socket implementations belong in **ESPressio Sockets**.

ESP-NOW implementations belong in **ESPressio ESP-Now**.

Hardware-radio implementations belong in the planned **ESPressio Radio** library.

---

# Summary

ESPressio Serial 0.1.0 establishes the Serial/console diagnostics layer of the ESPressio ecosystem.

The initial architecture is:

```text
ESPressio Event 5.5
        |
        | Event Transport Transaction Observation
        v
ESPressio Serial
        |
        | EventMonitor
        v
Arduino Print
        |
        +--> Hardware Serial
        +--> USB CDC
        +--> other Print destinations
```

The core Serial library remains dependency-free with respect to other ESPressio libraries, while Event monitoring is explicitly opt-in.
