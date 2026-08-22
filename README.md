# ESPressio Serial

Serial, console, logging and diagnostics components for the Flowduino ESPressio Development Platform.

ESPressio Serial is intentionally the **terminal/operator layer** of the ecosystem. It provides human-facing diagnostics and interactive control without forcing Serial concerns into the libraries being observed.

## Current Version — 0.7.2

Version **0.7.2** is a dependency-maintenance patch release over 0.7.1. It raises Serial's validated optional ESP-Now monitor baseline to released ESP-Now 0.8.0, preserving all Serial public interfaces and runtime semantics. Serial's core still has no mandatory ESPressio dependencies; integrations are selected explicitly.

- structured logs;
- a bounded diagnostic history before a crash/fault;
- an interactive console;
- Command-backed operator controls;
- runtime Event discovery/composition;
- live Timing/Threads/Event/Security/Sockets/ESP-NOW diagnostics.

ESPressio Serial provides those facilities without changing the semantics of the subsystem being observed.

```text
ESPressio subsystem
       |
       | synchronous observer / existing API
       v
Serial monitor / console
       |
       v
Arduino Print / Stream
       |
   Serial / USB CDC / compatible endpoint
```

## ESPressio Development Platform

ESPressio libraries are designed to be light-weight, composable, object-oriented and explicit about dependency direction. Serial follows the “pay only for what you select” rule: optional monitors and consoles acquire their source libraries only when those headers/features are used.

## License

Apache License 2.0. See [LICENSE](LICENSE).

## Namespace

```cpp
ESPressio::Serial
```

Because Arduino exposes a global object named `Serial`, fully qualified ESPressio Serial names are recommended:

```cpp
ESPressio::Serial::EventMonitor monitor;
```

while the Arduino hardware port remains:

```cpp
::Serial
```

# Dependency model

The **core ESPressio Serial library has no required ESPressio dependencies**.

Optional integrations are:

```text
CommandConsole / CommandMonitor
    - - -> ESPressio Command >= 1.0.0 < 2.0.0

SecurityMonitor
    - - -> ESPressio Security >= 0.3.0 < 1.0.0

SocketWorkerMonitor
    - - -> ESPressio Sockets >= 0.7.0 < 1.0.0

SocketSecuritySessionMonitor
    - - -> ESPressio Sockets >= 0.7.0 < 1.0.0
    - - -> ESPressio Security >= 0.3.0 < 1.0.0

ESPNowTransportMonitor
    - - -> ESPressio ESP-Now >= 0.8.0 < 1.0.0

SystemClockMonitor
    - - -> ESPressio Timing >= 2.2.4 < 3.0.0

ThreadMonitor
    - - -> ESPressio Threads >= 3.1.4 < 4.0.0

EventMonitor / EventConsole
    - - -> ESPressio Event >= 6.0.0 < 7.0.0
    - - -> ESPressio Serializable >= 0.10.2 < 1.0.0
```

The coordinated release generation is:

```text
Observable    3.0.1
Serializable  0.10.2
Units         0.2.3
Timing        2.2.4
Threads       3.1.4
Command       1.0.0
Security      0.3.0
Event         6.0.0
Sockets       0.7.0
ESP-Now       0.8.0
Serial        0.7.2
```

Command 1.0.0 introduces representation-neutral `CommandValue` values for structured invocations. Serial's Command monitor/console integrations are validated directly against that 1.x API. Sockets 0.7.0 preserves its existing Command protocol-v1 wire format while adapting typed values at its transport boundary. ESP-Now 0.8.0 retains the same wire/protocol compatibility as 0.7.x while hardening its receive-task stack and exposing stack-headroom diagnostics.

Event 6.0.0 contains only the generic Event mechanism plus integrations for libraries Event genuinely consumes itself. Command-, Security-, Sockets-, and ESP-Now-specific Event integrations remain supplied by their owning libraries, with Serial downstream of the complete graph.

# Interactive Runtime Serializable Event Console

Serial 0.7.2 retains the structured EventMonitor safety introduced in 0.5.1: bounded, allocation-free ESPB traversal from ESPressio Serializable 0.10.2 with fail-safe hexadecimal fallback for malformed or outside-limit payloads.

Architecture:

```text
operator
   |
   v
ESPressio Serial Console
   |
   | JSON
   v
ESPressio Serializable JsonArchive
   |
   | SerializationNode
   v
EventTransportManager runtime registry/factory
   |
   v
concrete Serializable Event
   |
   v
normal Queue / Stack dispatch
   |
   +--> local listeners
   |
   +--> configured Event transports
```

Serial does not create a second Event registry or a transport-specific remote-dispatch mechanism.

## Initialize `EventConsole`

```cpp
#include <ESPressio_EventConsole.hpp>

ESPressio::Serial::Console console;
ESPressio::Serial::EventConsole eventConsole;

void setup() {
    ::Serial.begin(115200);

    console.Initialize(::Serial, ::Serial);
    eventConsole.Initialize(console);
}
```

The Event integration requires Event 6.0.x and Serializable 0.10.x.

## Safe-by-default Event authorization

Runtime discovery does **not** automatically imply operator permission to dispatch every registered Event.

The default policy is allow-list oriented. Explicitly authorize an Event type:

```cpp
eventConsole.AllowEvent<CameraShutterEvent>();
```

or by stable wire name:

```cpp
eventConsole.AllowEvent(
    "flowduino.motor.move.v1"
);
```

A controlled development environment can explicitly allow all registered types, while deny-list entries can still override broad access.

This prevents a destructive administrative Event from becoming operator-triggerable merely because it was registered with Event Transport.

## Discover registered Events

At the console:

```text
events
```

or:

```text
event list
```

lists runtime-registered Serializable Event types, including useful schema/routing/constructibility/access information.

Describe one type:

```text
event describe flowduino.motor.move.v1
```

The description comes from Event/Serializable runtime metadata rather than a duplicate Serial-maintained schema.

## One-line JSON dispatch

Queue:

```text
event queue flowduino.motor.move.v1 {"axis":"pan","position":45,"speed":20}
```

Stack:

```text
event stack flowduino.motor.move.v1 {"axis":"pan","position":45,"speed":20}
```

JSON is parsed into a representation-neutral `SerializationNode`, then Event's normal runtime factory applies schema migration, aliases, defaults, required fields, numeric constraints and validators.

## Interactive composition

```text
event compose flowduino.motor.move.v1
```

can enter an interactive one-line payload flow. The enclosing Console and EventConsole both apply configured size bounds.

## Validation diagnostics

Construction failures expose the ordinary Serializable diagnostics, including property path, error code and message. Serial does not invent a separate validation layer.

## Confirmation and dispatch controls

Confirmation is enabled by default so an operator must explicitly accept a dispatch. Queue and Stack can also be enabled/disabled independently through `EventConsoleConfig`.

## Audit logging

`EventConsole` can send operator/security audit records to an existing `ILoggerSink`. Useful audit conditions include successful dispatch, denied dispatch, malformed JSON, unknown Event type, validation failure and dispatch failure.

Payload content is not blindly duplicated into audit logs, avoiding accidental disclosure of sensitive values.

# Logging

The logging layer separates a log record from where it is displayed or retained.

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

Multiple sinks may be active simultaneously. Serial output is one sink, not the logging architecture itself.

A compile-time log-level configuration can remove lower-severity delivery paths while runtime filtering remains available through `SetMinimumLevel()`.

# Diagnostic flight recorder

`DiagnosticRingBuffer<Capacity>` is both a log sink and bounded in-memory history:

```cpp
ESPressio::Serial::DiagnosticRingBuffer<64> history;
logger.AddSink(history);

// After a fault or operator request:
history.Dump(::Serial);
```

The oldest entry is overwritten when capacity is exhausted. This makes the ring buffer suitable for retaining the diagnostic context immediately preceding a failure without unbounded memory growth.

# `SystemClockMonitor`

```cpp
#include <ESPressio_SystemClockMonitor.hpp>

ESPressio::Serial::SystemClockMonitor<> clockMonitor;
clockMonitor.Initialize(::Serial);
```

This monitor directly consumes ESPressio Timing's synchronous SystemClock observer surface. It reports meaningful time-setting, synchronization acceptance/rejection, synchronization-state, reset/configuration and callback scheduling/execution transitions.

It does not require Event merely to display Timing diagnostics.

# `ThreadMonitor`

```cpp
#include <ESPressio_ThreadMonitor.hpp>

ESPressio::Serial::ThreadMonitor threadMonitor;
threadMonitor.Initialize(::Serial);
```

`ThreadMonitor` observes process-wide ESPressio Threads infrastructure such as ThreadManager, garbage collection and termination dispatch lifecycle.

Again, the monitor consumes Threads' native Observer surface rather than routing diagnostics through Event unnecessarily.

# `EventMonitor`

`EventMonitor` observes Event Transport transactions. It is not itself an Event Transport and it does not alter routing.

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
```

The monitor can therefore show outbound/inbound Event transport activity regardless of whether the concrete transport is ESP-NOW, UDP, TCP, WebSocket or another implementation.

## Structured payload safety

Structured Event payload diagnostics use bounded, allocation-free ESPB traversal from Serializable 0.10.2.

Limits cover nesting depth, aggregate node count, collection size, names and strings. Malformed, truncated or outside-limit payloads fall back to bounded hexadecimal output instead of turning diagnostics into a fatal allocation/crash path.

This is particularly important because diagnostic code must not make a stressed embedded system less reliable.

# Other subsystem monitors

Serial 0.7.2 also provides opt-in monitors for:

```text
CommandMonitor
SecurityMonitor
SocketWorkerMonitor
SocketSecuritySessionMonitor
ESPNowTransportMonitor
```

These observe the originating library's native Observable contract. Security and Socket monitors are instance-oriented because the application chooses the concrete `TransportSecurity`, `SocketWorker` or `SocketSecuritySession` instance to observe.

`ESPNowTransportMonitor` observes the process-wide ESP-NOW transport lifecycle and peer/send activity. Serial 0.7.2 validates this monitor against ESPressio ESP-Now 0.8.0.

# Aggregate `DiagnosticMonitor`

Where the corresponding dependencies are present, `DiagnosticMonitor` can compose selected subsystem monitors behind one configuration surface.

```cpp
#include <ESPressio_DiagnosticMonitor.hpp>

ESPressio::Serial::DiagnosticMonitor diagnostics;

void setup() {
    ::Serial.begin(115200);

    ESPressio::Serial::DiagnosticMonitorConfig config;
    config.SystemClock = true;
    config.Threads = true;
    config.Events = true;

    diagnostics.Initialize(::Serial, config);
}
```

The aggregate uses compile-time feature availability; it does not make every supported ESPressio library a mandatory package dependency.

# Examples

The repository's `examples/` directory contains application-shaped examples for the generic Console, EventConsole, loopback EventConsole/monitoring, logging and supported monitor integrations.

The EventConsole loopback pattern is particularly useful for understanding the complete operator path:

```text
Serial JSON
    -> runtime Event
    -> local dispatch
    -> Event Transport
    -> inbound reconstruction
    -> EventMonitor output
```

# Design principles

- Serial is an operator/diagnostics layer, not a replacement for source-library APIs.
- Monitoring should observe the originating subsystem directly wherever possible.
- Interactive Event tooling should reuse Event/Serializable registries and validation.
- Core Serial remains dependency-free.
- Optional integrations remain opt-in and downstream.
- Diagnostic buffers and parsing limits are bounded for embedded reliability.

# Changelog

See [CHANGELOG.md](CHANGELOG.md) for release history and notable changes.
