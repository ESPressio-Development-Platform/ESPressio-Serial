# ESPressio Serial

Serial and console-oriented components for the Flowduino ESPressio Development Platform.

Version 0.5.2 hardens the generic Console under heap pressure by reserving its configured bounded input capacity during initialization, so normal line polling no longer grows the backing `std::string` incrementally. It retains the EventMonitor safety work from 0.5.1 and the Observable-backed monitors introduced in 0.5.0 while preserving the dependency-free Serial core.

## Current Version — 0.5.2

Version **0.5.2** fixes a low-memory failure path reproduced on ESP32 where `Console::Poll()` could reach `std::terminate()` when `std::string::push_back()` needed to grow the input buffer after the system was already under severe heap pressure. `Console::Initialize()` now prepares the configuration and reserves `ConsoleConfig::MaximumLineLength` before publishing the console as initialized. If that bounded capacity cannot be reserved, initialization returns `false` cleanly.

The EventMonitor structured-payload hardening from 0.5.1 remains unchanged: structured diagnostics use ESPressio Serializable 0.10.2's allocation-free BinaryArchive traversal API with explicit depth, aggregate-node, collection, name, and string limits. Invalid or outside-limit payloads fall back to bounded hexadecimal output rather than becoming fatal diagnostic work.

The Observable-backed monitor integrations introduced in 0.5.0 remain available unchanged:

```text
CommandMonitor
    - - -> ESPressio Command >= 0.3.0 < 1.0.0

SecurityMonitor
    - - -> ESPressio Security >= 0.2.0 < 1.0.0

SocketWorkerMonitor
    - - -> ESPressio Sockets >= 0.5.0 < 1.0.0

SocketSecuritySessionMonitor
    - - -> ESPressio Sockets >= 0.5.0 < 1.0.0
    - - -> ESPressio Security >= 0.2.0 < 1.0.0

ESPNowTransportMonitor
    - - -> ESPressio ESP-Now >= 0.5.2 < 1.0.0
```

`DiagnosticMonitor` can additionally compose `CommandMonitor` and `ESPNowTransportMonitor` when those dependencies are present. Security and Socket monitors remain instance-oriented because the application must choose the specific `TransportSecurity`, `SocketWorker`, or `SocketSecuritySession` object to observe.

These monitors subscribe directly to the originating library's Observable contract. They do not invent parallel Serial lifecycle semantics and do not require ESPressio Event. Event-backed observation remains a separate opt-in integration in ESPressio Event 5.8.2.

Historical documentation for earlier release generations remains below where useful.

Current coordinated dependency baselines for the 0.5.2 release are Units 0.2.3, Timing 2.2.4, Threads 3.1.4, ESP-Now 0.5.2, Event 5.8.2, and Serializable 0.10.2. Command 0.3.0, Security 0.2.0, and Sockets 0.5.0 remain the current optional integration baselines.

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
ESPressio Event >= 5.8.2 < 6.0.0
ESPressio Serializable >= 0.10.2 < 1.0.0
```

The additional opt-in monitoring dependencies for the 0.5.x line are listed above. Historical sections below retain older release-specific baselines where those versions are part of the documented history.

For the complete ecosystem hierarchy, see:

**[ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.md)**

In the dependency chart:

- **Solid relationships** represent required dependencies.
- **Dashed relationships** represent opt-in dependencies introduced only when the associated feature/header is used.

---




## Version 0.3.1 — Event 5.6.1 compatibility

Version 0.3.1 updates the optional `EventConsole` integration baseline to **ESPressio Event 5.6.1**.

Event 5.6.1 corrects `EventDispatchContext` equality semantics required by ESPressio Threads 3.1 `ReadWriteMutex<T>` change detection. No ESPressio Serial console, monitoring, logging, or EventConsole public API changes are required.

Applications using `EventConsole` should therefore target:

```ini
flowduino/ESPressio-Event@^5.7.1
```

The core Serial library and generic `Console` remain independent of ESPressio Event.

---

# Version 0.3.0 — Interactive Runtime Serializable Event Console

Version 0.3.0 adds the interactive operator/service-console layer.

The architecture deliberately preserves library ownership:

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
ESPressio Event 5.6 runtime registry/factory
   |
   v
concrete Serializable Event
   |
   v
normal Queue / Stack dispatch
   |
   +--> local listeners
   |
   +--> EventTransportManager
            |
            +--> any configured outbound transport
```

Serial does not create a second Event registry or remote-dispatch mechanism.

## Generic `Console`

The generic console is available independently of Event:

```cpp
#include <ESPressio_Console.hpp>

ESPressio::Serial::Console console;

void setup() {
    ::Serial.begin(115200);

    ESPressio::Serial::ConsoleConfig config;
    config.Prompt = "espressio> ";

    if (!console.Initialize(
        ::Serial,
        ::Serial,
        config
    )) {
        // The bounded input buffer could not be reserved.
        return;
    }

    console.RegisterCommand(
        "hello",
        "Print a greeting",
        [](const auto& context) {
            // Handle context.Arguments.
        }
    );
}

void loop() {
    console.Poll();
}
```

Input uses Arduino `Stream`; output uses Arduino `Print`.

The console therefore works with Hardware Serial, USB CDC, or another compatible implementation.

The line buffer is bounded through:

```cpp
ConsoleConfig::MaximumLineLength
```

Beginning with 0.5.2, that bounded capacity is reserved during `Initialize()`. A successful initialization therefore guarantees that ordinary input up to `MaximumLineLength` does not need to grow the backing line buffer while `Poll()` is running. If the reservation cannot be satisfied, initialization returns `false` and the console remains uninitialized.

The console supports:

```text
command registration
command unregistration
help
arguments
prompt configuration
optional input echo
multiple interactive line interceptors
backspace/delete handling
CR/LF handling
```

Multiple line interceptors are intentional: future console extensions can maintain independent interactive states without replacing one global input handler.

## `EventConsole`

The Event Console is opt-in:

```cpp
#include <ESPressio_EventConsole.hpp>
```

and requires:

```text
ESPressio Event >= 5.7.1
ESPressio Serializable >= 0.10.0 < 1.0.0
ArduinoJson (through the optional Serializable JsonArchive)
```

Initialize it over an existing `Console`:

```cpp
ESPressio::Serial::Console console;
ESPressio::Serial::EventConsole eventConsole;

console.Initialize(
    ::Serial,
    ::Serial
);

eventConsole.Initialize(
    console
);
```

## Safe-by-default Event authorization

Runtime Event discovery does **not** imply permission to dispatch an Event.

The default access policy is:

```cpp
EventConsoleAccessPolicy::AllowListedOnly
```

Allow specific Event types:

```cpp
eventConsole.AllowEvent<
    CameraShutterEvent
>();

eventConsole.AllowEvent(
    "flowduino.motor.move.v1"
);
```

For a controlled development environment, explicitly enable all registered types:

```cpp
eventConsole.SetAccessPolicy(
    ESPressio::Serial::
        EventConsoleAccessPolicy::
            AllRegistered
);
```

Deny-list entries override allow-all:

```cpp
eventConsole.DenyEvent<
    FactoryResetEvent
>();
```

This prevents a registered administrative/destructive Event from becoming operator-dispatchable merely because a console is enabled.

## Event discovery

List runtime-registered Serializable Events:

```text
espressio> events

Registered Serializable Events:
  flowduino.camera.shutter.v1 [constructible] [allowed] schema=1 defaultRouting=Outbound
  flowduino.motor.move.v1 [constructible] [allowed] schema=2 defaultRouting=Bidirectional
  flowduino.system.factory-reset.v1 [constructible] [denied] schema=1 defaultRouting=None
```

The equivalent command is:

```text
event list
```

## Event schema description

```text
espressio> event describe flowduino.motor.move.v1
```

uses Event 5.6's runtime descriptor and Serializable schema metadata to report:

```text
stable Event type name
stable Event type ID
schema version
runtime constructibility
operator access
default Event Transport direction
property names
property types
required state
read-only state
sensitive metadata
default-value availability
aliases
```

Per-transport route names are not fabricated: Event 5.6 currently exposes the default routing direction through the public runtime descriptor.

## One-line JSON dispatch

Queue:

```text
event queue flowduino.motor.move.v1 {"axis":"pan","position":45,"speed":20}
```

Stack:

```text
event stack flowduino.motor.move.v1 {"axis":"pan","position":45,"speed":20}
```

`event dispatch` is a Queue alias.

JSON is parsed through ESPressio Serializable's `JsonArchive`, converted to a representation-neutral `SerializationNode`, and passed to Event 5.6's runtime factory.

## Interactive composition

```text
event compose flowduino.motor.move.v1
```

or:

```text
event compose flowduino.motor.move.v1 stack
```

prompts for a one-line JSON object:

```text
Enter one-line JSON object for flowduino.motor.move.v1 (or 'cancel'):
{"axis":"pan","position":45,"speed":20}
```

## Serializable validation diagnostics

Runtime-created Events use the normal ESPressio Serializable validation path.

Validation errors are presented to the operator with:

```text
property path
serialization error code
diagnostic message
```

For example:

```text
Event payload validation failed with 2 issue(s):
  speed: NumericOutOfRange - Property failed its numeric range constraint
  axis: UnknownEnumValue - Value is not a registered enum mapping
```

No separate Serial-specific Event validation system exists.

## Confirmation

Confirmation is enabled by default:

```text
Dispatch Event 'flowduino.motor.move.v1' via Queue priority=Normal? [y/N]
```

Only `y` or `yes` proceeds; any other response cancels the dispatch.

It can be disabled explicitly:

```cpp
EventConsoleConfig config;
config.RequireConfirmation = false;
```

## Dispatch semantics

Event Console uses Event 5.6's ownership-safe runtime dispatch API.

Once dispatched, the Event follows the normal Event system:

```text
runtime-created Event
        |
        v
Queue / Stack
        |
        v
local Event dispatch
        |
        v
EventTransportManager
        |
        v
existing per-transport outbound routing
```

Event Console therefore knows nothing about ESP-NOW, UDP, TCP, WebSocket, MQTT, or another concrete Event transport.

## Audit logging

`EventConsole` can optionally send security/operation audit records to any existing:

```cpp
ILoggerSink
```

using:

```cpp
eventConsole.SetAuditSink(
    &history
);
```

Useful audit conditions include:

```text
successful operator dispatch
denied dispatch
unregistered type
malformed JSON
oversized JSON
construction/validation failure
dispatch failure
```

The Event payload itself is deliberately not copied into the audit message by default, avoiding accidental logging of sensitive properties.

## Event Monitor integration

Console-created Events naturally flow through the ordinary Event Transport pipeline.

If `EventMonitor` is enabled, the same operator-created Event appears in its normal outbound/inbound transaction diagnostics without any special integration code.

## Limits

Operator JSON is bounded by:

```cpp
EventConsoleConfig::MaximumJsonLength
```

and the enclosing generic Console independently bounds total input line length.

Queue and Stack dispatch can be independently disabled:

```cpp
config.AllowQueue = true;
config.AllowStack = false;
```

## Examples

Version 0.3.0 adds:

```text
examples/
├── Console/
│   └── Console.ino
│
├── EventConsole/
│   └── EventConsole.ino
│
└── EventConsoleLoopback/
    └── EventConsoleLoopback.ino
```

`EventConsoleLoopback` combines the operator console, Event Console, Event Monitor, Serializable Event, and a local loopback `IEventTransport` to demonstrate the complete:

```text
Serial JSON
    -> runtime Event
    -> local dispatch
    -> Event Transport
    -> inbound reconstruction
    -> Serial Event Monitor
```

pipeline on one ESP32.

## Tests

The repository includes host-side tests for:

```text
generic Console command dispatch
argument preservation
multiple interactive line interceptors
interceptor removal
Stream polling
bounded Console input capacity reservation
capacity retention across polling and line clearing
over-length discard handling without buffer growth
runtime Event listing
Event schema description
allow-list enforcement
JSON command processing
pre-dispatch confirmation
type-erased dispatch
```

The Event Console contract test uses narrow test doubles for Event 5.6 and the Serializable JSON adapter, while release preparation verifies compatibility with the real public API surface.

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

This integration directly consumes ESPressio Timing 2.2.2's `ISystemClockObserver` notifications. It reports time-setting, synchronization acceptance/rejection, synchronization state changes, resets/configuration changes, and callback scheduling/execution.

Synchronization output includes the clock value before correction, the value after correction, and the immediate nanosecond difference.

This is an opt-in Timing dependency; ESPressio Event is not involved.

## Thread Monitor

```cpp
#include <ESPressio_ThreadMonitor.hpp>

ESPressio::Serial::ThreadMonitor threadMonitor;

threadMonitor.Initialize(::Serial);
```

`ThreadMonitor` directly observes the process-wide ESPressio Threads 3.1.2 infrastructure:

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

In 0.5.0, `DiagnosticMonitorConfig` additionally supports `Commands` and `ESPNow`. These remain disabled by default and are available only when the corresponding optional upstream headers are present.

## Dependency model

```text
ESPressio Serial core
    -> no mandatory ESPressio dependency

Logging
    -> no additional ESPressio dependency

SystemClockMonitor
    - - -> ESPressio Timing >= 2.2.2 < 3.0.0

ThreadMonitor
    - - -> ESPressio Threads >= 3.1.2 < 4.0.0

EventMonitor
    - - -> ESPressio Event >= 5.8.0 < 6.0.0
    - - -> ESPressio Serializable >= 0.10.1 < 1.0.0
```

All ESPressio relationships remain opt-in. The 0.5.0 observer monitors add the additional optional relationships documented near the top of this README.

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

ESPressio Event Transport serializes Event payloads using ESPressio Serializable's BinaryArchive ESPB v2 representation.

Beginning with Serial 0.5.1, EventMonitor does **not** decode that payload into a second `SerializationNode` tree merely for presentation. Instead, it uses Serializable 0.10.2's `TraverseBinaryArchive()` API to validate and stream the existing ESPB bytes directly to the selected Arduino `Print` destination.

This keeps human-readable structured diagnostics independent of the concrete C++ Event type and avoids ArduinoJson, while removing duplicate payload-tree allocations from the synchronous Event Transport observer path.

If the payload is malformed or exceeds the configured diagnostic limits, EventMonitor prints a bounded hexadecimal fallback rather than attempting structured tree construction.

---

# Structured-output limits

Diagnostic output should not be allowed to grow without bound.

Configuration includes:

```cpp
MaximumCollectionItems
MaximumStringLength
MaximumStructuredNodes
MaximumStructuredDepth
IndentSpaces
PrettyStructuredPayload
```

These limits are applied while validating/traversing ESPB bytes before structured output is emitted. `MaximumStructuredNodes` bounds aggregate payload-tree breadth as well as the existing collection/string/depth controls.

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

ESPressio Event transaction snapshots expose borrowed Event/payload references valid only during the Observer callback.

`EventMonitor` consumes those values synchronously and does not retain borrowed transaction pointers after the callback returns.

Structured traversal is therefore performed while the payload is valid, without copying it into a second tree.

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

It defines a Serializable counter Event, transports it through Event, and renders the Binary payload as structured text.

---

# PlatformIO

A project using only the core Serial library:

```ini
lib_deps =
    flowduino/ESPressio-Serial@^0.5.2
```

An application using Event Monitor requires:

```ini
lib_deps =
    flowduino/ESPressio-Serial@^0.5.2
    flowduino/ESPressio-Event@^5.8.2
    flowduino/ESPressio-Serializable@^0.10.2
```

The Event/Serializable dependencies are intentionally not declared as mandatory package dependencies of ESPressio Serial because they are required only by the opt-in Event Monitor feature.

---


## PlatformIO: Event Console

The generic console requires only ESPressio Serial:

```ini
lib_deps =
    flowduino/ESPressio-Serial@^0.5.2
```

The Event Console additionally requires the runtime Event and JSON stacks:

```ini
lib_deps =
    flowduino/ESPressio-Serial@^0.5.2
    flowduino/ESPressio-Event@^5.8.2
    flowduino/ESPressio-Serializable@^0.10.2
    bblanchon/ArduinoJson
```

ArduinoJson is required only because `EventConsole` selects ESPressio Serializable's optional `JsonArchive`; it remains unnecessary for core Serial, logging, diagnostics, and the generic Console.

# Future direction

ESPressio Serial is intended to contain Serial/console-oriented ESPressio integrations rather than becoming a general communications catch-all.

Potential future components include:

```text
Serial Event Transport
structured Event-based remote log sinks
persistent diagnostic sinks
additional subsystem console commands
serial configuration interfaces
serial protocol adapters
operator authentication/session policy where appropriate
```

Network/socket implementations belong in **ESPressio Sockets**.

ESP-NOW implementations belong in **ESPressio ESP-Now**.

Hardware-radio implementations belong in the planned **ESPressio Radio** library.

---

# Summary

ESPressio Serial provides three complementary layers:

```text
CORE
    ESPressio_Serial.hpp
    diagnostic types
    no mandatory ESPressio dependency

DIAGNOSTICS / LOGGING
    Logger
    SerialLogSink
    DiagnosticRingBuffer
    SystemClockMonitor          [opt-in Timing]
    ThreadMonitor               [opt-in Threads]
    EventMonitor                [opt-in Event + Serializable]
    Command/Security/Sockets/ESP-Now monitors [opt-in]
    DiagnosticMonitor

OPERATOR CONSOLE
    Console
        Stream input
        Print output
        extensible commands

    EventConsole                [opt-in Event + Serializable JSON]
        runtime Event discovery
        schema description
        JSON composition
        validation diagnostics
        allow/deny policy
        confirmation
        Queue / Stack dispatch
```

The central rule remains unchanged:

**ESPressio Serial owns human/operator interaction and presentation; the upstream ESPressio libraries continue to own their underlying runtime semantics.**


## ESPressio Command Integration (0.4.0)

Serial 0.4.0 adds an opt-in bridge to **ESPressio Command >= 0.2.0 < 1.0.0**. Core Serial remains usable without Command. Include `ESPressio_CommandConsole.hpp` only when the integration is required.

```ini
lib_deps =
    flowduino/ESPressio-Serial@^0.4.0
    flowduino/ESPressio-Command@^0.2.0
```

```cpp
#include <ESPressio_Console.hpp>
#include <ESPressio_CommandConsole.hpp>
#include <ESPressio_Commands.hpp>

ESPressio::Serial::Console console;
ESPressio::Serial::CommandConsole commandConsole;

void setup() {
    console.Initialize(Serial, Serial);
    commandConsole.Initialize(console);

    auto& commands = ESPressio::Command::CommandRegistry::GetInstance();
    commands.Command("system").Command("status")
        .OnExecute([](const ESPressio::Command::CommandContext&) {
            return ESPressio::Command::CommandResult::Ok("System OK");
        });
}

void loop() { console.Poll(); }
```

`CommandConsole` reuses Serial's existing input/prompt handling and forwards resolvable lines into the shared transport-neutral Command registry. Unknown roots fall through so other Console interceptors and legacy commands can continue to coexist.

### EventConsole on the shared Command tree

When Command integration is selected, initialize EventConsole with `CommandConsole`:

```cpp
ESPressio::Serial::EventConsole eventConsole;
eventConsole.Initialize(commandConsole);
```

EventConsole then registers the following shared Command tree with ownership-safe registration handles:

```text
event list
event describe <type>
event compose <type> [queue|stack]
event queue <type> <json>
event stack <type> <json>
event dispatch <type> <json>
event cancel
events
```

Shutdown removes the registered subtrees, preventing callbacks from outliving the EventConsole instance. The previous `Initialize(Console&, ...)` overload remains available for compatibility with existing applications.
