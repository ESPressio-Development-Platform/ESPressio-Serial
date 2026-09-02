# ESPressio Serial

Serial, console and operator-diagnostics components for the ESPressio Development Platform.

ESPressio Serial is intentionally the **terminal/operator layer** of the ecosystem. It observes and controls other ESPressio subsystems without forcing Serial concerns back into those libraries. Generic logging concepts are owned by `ESPressio-Logging`; this library owns only the concrete Serial Logging Sink.

## Current Version — 0.8.1

0.8.1 is the current released baseline. This feature branch does not change version numbering. It removes the former Serial-owned Logger, logging record types, Sink abstraction and diagnostic ring buffer, and consumes the new `ESPressio-Logging` abstraction instead.

## Namespace

```cpp
ESPressio::Serial
```

Because Arduino exposes a global object named `Serial`, fully qualified ESPressio names remain recommended in ESP32 applications.

# Platform-neutral byte I/O

Core Serial does not store Arduino `Stream` or `Print` objects. Console input/output and the Serial Logging Sink consume:

```cpp
ESPressio::System::IO::IByteInput
ESPressio::System::IO::IByteOutput
ESPressio::System::IO::IByteStream
```

Framework-specific byte transport remains in the platform layer.

During coordinated development:

```ini
lib_deps =
    https://github.com/ESPressio-Development-Platform/ESPressio-System.git#main
    https://github.com/ESPressio-Development-Platform/ESPressio-Logging.git#main
    https://github.com/ESPressio-Development-Platform/ESPressio-Serial.git#feature/40-logging-sink
```

On Arduino-ESP32, add ESPressio-ESP32 and create the adapter at the application boundary:

```cpp
#include <ESPressio_ArduinoByteStream.hpp>
#include <ESPressio_Console.hpp>

ESPressio::ESP32Platform::ArduinoByteStream consoleIO(::Serial);
ESPressio::Serial::Console console;

void setup() {
    ::Serial.begin(115200);
    console.Initialize(consoleIO);
}
```

Separate input/output adapters are also available when the two directions use different framework objects.

# Interactive console

`Console` owns bounded line collection, optional echo, prompt rendering, line interception, command registration and help/error output. It can be initialized with one bidirectional byte stream:

```cpp
console.Initialize(consoleIO);
```

or distinct portable input/output endpoints:

```cpp
console.Initialize(input, output);
```

The console remains independent of the mechanism carrying those bytes. Arduino UART, USB serial, a test stream or another platform adapter can satisfy the same contract.

`CommandConsole` integrates ESPressio Command with this portable console. Domain-owned Command handlers automatically become available once registered with the shared `CommandRegistry`:

```text
operator -> byte stream -> Serial Console -> CommandRegistry -> domain Command handler
```

`EventConsole` provides runtime discovery and composition/dispatch of registered Serializable Events while reusing Event's normal registry, authorization and validation mechanisms. Its audit messages now route through the central ESPressio Logger rather than through a Serial-owned logging abstraction.

# Logging Sink

`ESPressio-Logging` owns `Logger`, `LogRouter`, `LogRecordView`, `LogRecordLease`, levels, categories, metadata and `ILogSink`. ESPressio Serial no longer duplicates any of those concepts.

Include `ESPressio_SerialLogging.hpp` when consuming the Serial Sink. This deliberately has a distinct name from the generic `ESPressio_Logging.hpp` umbrella owned by ESPressio-Logging, avoiding ambiguous/self-shadowing headers.

`SerialLogSink` is the concrete adapter from an ESPressio Logging record to a portable `IByteOutput`:

```cpp
#include <ESPressio_SerialLogging.hpp>
#include <ESPressio_ArduinoByteStream.hpp>

inline constexpr auto ApplicationCategory =
    ESPressio::Logging::LogCategory::Named("Application");

ESPressio::ESP32Platform::ArduinoByteOutput serialOutput(::Serial);
ESPressio::Serial::SerialLogSink serialSink(serialOutput);

void setup() {
    ESPressio::Logging::Logger::GetInstance()
        .Router()
        .RegisterSink(&serialSink);

    ESPRESSIO_LOG_INFO(ApplicationCategory, "Boot complete");
}
```

The Sink executes synchronously on the informing thread, retains no `LogRecordLease`, and writes the supplied message/category/metadata views directly to the byte-output abstraction. Numeric formatting uses only bounded stack-local buffers. No owning log string or serialized intermediate representation is constructed. A Sink-local mutex serializes complete records so concurrent callers cannot interleave output fragments, while its level mask is atomically readable/writable without taking that output lock.

Default output is compact but preserves both Logging timestamps where available:

```text
[mono=123456789ns system=1700000000000000000ns] [ERROR] [Laser-Trigger] triggered channel=6 armed=true
```

The Sink has its own independent `LogLevelMask`, allowing it to participate in Logging's per-Sink filtering without reintroducing Serial-specific severity types.

# WiFi diagnostics

`WiFiMonitor` observes ESPressio-WiFi's native `IWiFiObserver` surface. It does not poll Arduino WiFi directly and it never reads configuration credentials.

Typical output remains intentionally compact and keeps AP and Client contexts separate:

```text
[ESPressio WiFi] Mode ap -> ap-client
[ESPressio WiFi] AP starting -> active ssid=ESPressio-Lab stations=0
[ESPressio WiFi] Client connecting -> connected ssid=Studio rssi=-43 channel=6
[ESPressio WiFi] ClientIPAddressAcquired ip=192.168.1.42 gateway=192.168.1.1
```

The monitor consumes only ESPressio WiFi public types. Arduino/ESP-IDF WiFi types never cross the integration boundary.

## Credential safety

WiFi diagnostics deliberately have no API that reads or prints plaintext passwords. Runtime state such as SSID, RSSI, channel, IP address and station identity can be reported without exposing persisted credentials.

# Other monitors

Serial provides opt-in monitors for Timing/System Clock, Threads, Event Transport, Command registry activity, Security lifecycle/failures, Sockets, ESP-NOW and WiFi. Each monitor observes the originating subsystem's native Observer surface rather than inventing a parallel lifecycle model.

`EventMonitor` structured ESPB diagnostics use bounded, allocation-free traversal and fall back to bounded hexadecimal output for malformed or outside-limit payloads, keeping diagnostic code fail-safe on constrained devices.

# Dependency model

The Serial library now has two required ESPressio dependencies:

```text
Serial
    -> System    (portable byte I/O)
    -> Logging   (log record / router / Sink contract)
```

`Logging` in turn owns its own System/Observable/Timing dependencies. Serial does not duplicate those abstractions.

Optional integrations remain downstream and selected only when their corresponding headers/features are used:

```text
CommandConsole / CommandMonitor  - - -> Command
SecurityMonitor                  - - -> Security
Socket monitors                  - - -> Sockets
ESPNowTransportMonitor           - - -> ESP-Now
SystemClockMonitor               - - -> Timing
ThreadMonitor                    - - -> Threads
EventMonitor / EventConsole      - - -> Event / Serializable
WiFiMonitor                      - - -> WiFi
```

Serial remains terminal/downstream. No upstream domain library should depend on Serial merely to obtain Logging; upstream libraries consume `ESPressio-Logging` directly.

# Platform boundary

The intended ESP32 composition is:

```text
Arduino Serial / Stream / Print
             |
             v
ESPressio-ESP32 byte adapter
             |
             v
System::IO byte contract
             |
             v
ESPressio-Serial Console / SerialLogSink / diagnostics
```

Raw byte transport is generic hardware/runtime I/O and belongs in System. Logging semantics belong in ESPressio-Logging. Serial owns the operator-facing concrete representation of a log record on a Serial byte stream.

# Design principles

- Serial is an operator/diagnostics layer, not a replacement for source-library APIs.
- Core Serial remains framework- and platform-neutral.
- Generic Logging contracts belong to `ESPressio-Logging`, not Serial.
- `SerialLogSink` is synchronous, non-owning and does not retain borrowed Logging records.
- Complete Serial log records are serialized against concurrent callers; the Router itself still owns no execution thread.
- Framework byte-stream types are adapted at the platform/application boundary.
- Optional integrations remain opt-in and downstream.
- Monitors consume ESPressio public types rather than lower-framework implementation types.
- Sensitive configuration values are not emitted merely because diagnostics are enabled.
- Diagnostic parsing limits are bounded for embedded reliability.
- Command/Event operator surfaces reuse their authoritative registries and validation paths.

# Platform abstraction audit

See [PLATFORM_ABSTRACTIONS.md](PLATFORM_ABSTRACTIONS.md) for the migration record.

# Changelog

See [CHANGELOG.md](CHANGELOG.md) for release history and notable changes.

## License

Apache License 2.0. See [LICENSE](LICENSE).
