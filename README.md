# ESPressio Serial

Serial, console, logging and diagnostics components for the ESPressio Development Platform.

ESPressio Serial is intentionally the **terminal/operator layer** of the ecosystem. It observes and controls other ESPressio subsystems without forcing Serial concerns back into those libraries.

## Current Version — 0.8.1

0.8.1 is a dependency-maintenance release aligning every optional Serial integration with the completed released Serializable 0.11.3 cascade. Core Serial still has no mandatory ESPressio dependencies; every subsystem integration is selected explicitly.

Validated generation:

```text
Observable    3.0.2
Serializable  0.11.3
Units         0.2.7
Timing        2.2.8
Threads       3.1.7
Event         6.0.3
Command       1.0.3
Security      0.4.2
Persistence   0.3.2
Sockets       0.7.3
ESP-Now       0.8.3
WiFi          0.2.0
Serial        0.8.1
```

## Namespace

```cpp
ESPressio::Serial
```

Because Arduino exposes a global object named `Serial`, fully qualified ESPressio names are recommended:

```cpp
ESPressio::Serial::WiFiMonitor wifiMonitor;
```

while the hardware port remains `::Serial`.

# WiFi diagnostics

`WiFiMonitor` observes ESPressio WiFi's native `IWiFiObserver` surface. It does not poll Arduino WiFi directly and it never reads configuration credentials.

```cpp
#include <ESPressio_WiFiMonitor.hpp>

ESPressio::Serial::WiFiMonitor wifiMonitor;

void setup() {
    ::Serial.begin(115200);
    wifiMonitor.Initialize(::Serial, wifi);
}
```

Typical output is intentionally compact and keeps AP and Client contexts separate:

```text
[ESPressio WiFi] Mode ap -> ap-client
[ESPressio WiFi] AP starting -> active ssid=ESPressio-Lab stations=0
[ESPressio WiFi] Client connecting -> connected ssid=Studio rssi=-43 channel=6
[ESPressio WiFi] ClientIPAddressAcquired ip=192.168.1.42 gateway=192.168.1.1
[ESPressio WiFi] APStationConnected station=94:B5:55:19:1D:9C
```

An on-demand runtime snapshot is available without enabling periodic noise:

```cpp
wifiMonitor.PrintStatus(wifi);
```

which produces a line such as:

```text
[ESPressio WiFi] Status mode=ap-client ap=active stations=1 client=connected ip=192.168.1.42 scan=idle
```

## Scan diagnostics

When WiFi performs an asynchronous scan, the monitor reports lifecycle and results:

```text
[ESPressio WiFi] Scan idle -> scanning
[ESPressio WiFi] ScanComplete count=2
  ssid=Studio rssi=-43 channel=6 security=wpa2
  ssid=Guest rssi=-71 channel=11 security=open
```

The monitor consumes only ESPressio WiFi public types. Arduino/ESP-IDF WiFi types never cross the integration boundary.

## Credential safety

`WiFiMonitor` deliberately has no API that accepts or prints WiFi passwords. It observes runtime state such as SSID, RSSI, channel, IP address and station identity only.

A WiFi Command handler may set credentials, but neither that handler nor this monitor provides a plaintext credential-read operation. Persisted credentials should use ESPressio WiFi's protected Persistence integration.

# Dependency model

The **core ESPressio Serial library has no required ESPressio dependencies**.

Optional integrations include:

```text
CommandConsole / CommandMonitor
    - - -> Command >= 1.0.3 < 2.0.0

SecurityMonitor
    - - -> Security >= 0.4.2 < 1.0.0

SocketWorkerMonitor / SocketSecuritySessionMonitor
    - - -> Sockets >= 0.7.3 < 1.0.0

ESPNowTransportMonitor
    - - -> ESP-Now >= 0.8.3 < 1.0.0

SystemClockMonitor
    - - -> Timing >= 2.2.8 < 3.0.0

ThreadMonitor
    - - -> Threads >= 3.1.7 < 4.0.0

EventMonitor / EventConsole
    - - -> Event >= 6.0.3 < 7.0.0
    - - -> Serializable >= 0.11.3 < 1.0.0

WiFiMonitor
    - - -> WiFi >= 0.2.0 < 1.0.0
```

Serial remains terminal/downstream. No upstream library should depend on Serial.

# Interactive Command and Event tooling

`CommandConsole` integrates ESPressio Command with a `Stream`/`Print` console. Domain-owned Command handlers, including WiFi's optional `WiFiCommandHandler`, automatically become usable from that console once registered with the shared `CommandRegistry`.

```text
operator -> Serial Console -> CommandRegistry -> WiFiCommandHandler -> WiFiManager
```

This is deliberately transport-independent: a future Web console can invoke the same Command tree without changing WiFi.

`EventConsole` provides runtime discovery and JSON composition/dispatch of registered Serializable Events while reusing Event's normal registry, authorization and validation mechanisms.

```cpp
#include <ESPressio_EventConsole.hpp>

ESPressio::Serial::Console console;
ESPressio::Serial::EventConsole eventConsole;

void setup() {
    console.Initialize(::Serial, ::Serial);
    eventConsole.Initialize(console);
}
```

Useful operator commands include `events`, `event describe`, `event queue`, `event stack`, and `event compose`. Runtime Event authorization remains safe-by-default and should be explicitly configured by the application.

# Logging and diagnostic history

The logging layer separates records from sinks:

```cpp
#include <ESPressio_Logging.hpp>

ESPressio::Serial::Logger<> logger;
ESPressio::Serial::SerialLogSink serialSink(::Serial);
ESPressio::Serial::DiagnosticRingBuffer<64> history;

logger.AddSink(serialSink);
logger.AddSink(history);
logger.Info("Application", "Boot complete");
```

`DiagnosticRingBuffer` retains bounded pre-failure history and can later be dumped to any Arduino `Print` implementation.

# Other monitors

Serial provides opt-in monitors for Timing/System Clock, Threads, Event Transport, Command registry activity, Security lifecycle/failures, Sockets, ESP-NOW and WiFi. Each monitor observes the originating subsystem's native Observer surface rather than inventing a parallel lifecycle model.

`EventMonitor` structured ESPB diagnostics use bounded, allocation-free traversal and fall back to bounded hexadecimal output for malformed or outside-limit payloads, keeping diagnostic code fail-safe on constrained devices.

# Design principles

- Serial is an operator/diagnostics layer, not a replacement for source-library APIs.
- Core Serial remains dependency-free.
- Optional integrations remain opt-in and downstream.
- Monitors consume ESPressio public types rather than lower-framework implementation types.
- Sensitive configuration values are not emitted merely because diagnostics are enabled.
- Diagnostic buffers and parsing limits are bounded for embedded reliability.
- Command/Event operator surfaces reuse their authoritative registries and validation paths.
- CI and integration validation use released ESPressio tags only; development branches are not release dependencies.

# Changelog

See [CHANGELOG.md](CHANGELOG.md) for release history and notable changes.

## License

Apache License 2.0. See [LICENSE](LICENSE).
