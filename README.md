# ESPressio Serial

Serial and console-oriented diagnostics, logging and operator tooling for the Flowduino ESPressio Development Platform.

## Current Development Version

This branch targets **ESPressio Serial 0.5.0**.

0.5.0 extends Serial's diagnostics role with opt-in monitors for the new Observable lifecycle surfaces in ESPressio Command, Security, Sockets and ESP-Now. The originating library remains the source of truth; Serial only subscribes and renders those observations to an Arduino `Print` destination.

See [CHANGELOG.md](CHANGELOG.md) for release history.

## Dependency philosophy

The **core ESPressio Serial library remains dependency-free within the ESPressio ecosystem**. Optional facilities are selected explicitly by including their corresponding headers.

Current optional integration baselines are:

- **ESPressio Timing >= 2.2.2 and < 3.0.0** — System Clock monitoring.
- **ESPressio Threads >= 3.1.2 and < 4.0.0** — Thread Manager/GC/termination monitoring.
- **ESPressio Event >= 5.8.0 and < 6.0.0** — Event Monitor and EventConsole integrations.
- **ESPressio Serializable >= 0.10.0 and < 1.0.0** — runtime Serializable Event tooling where selected.
- **ESPressio Command >= 0.3.0 and < 1.0.0** — CommandConsole and `CommandMonitor`.
- **ESPressio Security >= 0.2.0 and < 1.0.0** — `SecurityMonitor`.
- **ESPressio Sockets >= 0.5.0 and < 1.0.0** — socket worker/security-session monitors.
- **ESPressio ESP-Now >= 0.5.0 and < 1.0.0** — `ESPNowTransportMonitor`.

No one of these is introduced as a mandatory dependency of `ESPressio_Serial.hpp`.

```text
                         +--> Timing monitor
                         +--> Threads monitor
ESPressio Serial core ---+--> Event monitor / EventConsole
                         +--> Command console / monitor
                         +--> Security monitor
                         +--> Sockets monitors
                         +--> ESP-Now monitor

(all relationships above are opt-in)
```

See [ESPRESSIO_DEPENDENCY_CHART.md](ESPRESSIO_DEPENDENCY_CHART.md) for the broader ecosystem relationship view.

## Core umbrella

```cpp
#include <ESPressio_Serial.hpp>
```

The core umbrella exposes common Serial/diagnostic types and documents optional entry points without batch-including their dependencies.

## Logging

The existing logging layer remains available through `ESPressio_Logging.hpp`, including log levels, pluggable sinks, `SerialLogSink`, filtering and retained diagnostic history.

## Existing monitors

Serial continues to provide:

- `SystemClockMonitor` — Timing System Clock observer output;
- `ThreadMonitor` — Thread Manager, Garbage Collector and Termination Dispatcher observations;
- `EventMonitor` — Event Transport transaction/event diagnostics; and
- `DiagnosticMonitor` — convenience aggregation of available monitors.

## Observable subsystem monitors

0.5.0 adds four new opt-in monitoring areas.

### Command

```cpp
#include <ESPressio_CommandMonitor.hpp>

ESPressio::Serial::CommandMonitor monitor;
monitor.Initialize(Serial);
```

`CommandMonitor` consumes `ICommandRegistryObserver` and prints command-root registration/unregistration lifecycle changes. It does not observe or intercept command execution itself.

### Security

```cpp
#include <ESPressio_SecurityMonitor.hpp>

ESPressio::Serial::SecurityMonitor monitor;
monitor.Initialize(Serial, security);
```

`SecurityMonitor` subscribes to a specific `TransportSecurity` instance and reports configuration changes, session reset/establishment, replay-protection reset and security failures. Key material is never rendered.

### Sockets

```cpp
#include <ESPressio_SocketWorkerMonitor.hpp>
#include <ESPressio_SocketSecuritySessionMonitor.hpp>
```

`SocketWorkerMonitor` subscribes to a specific `SocketWorker` and reports start/start-failure/stop transitions.

`SocketSecuritySessionMonitor` subscribes to a specific `SocketSecuritySession` and reports secure-session faults and explicit resets.

The instance-specific API is deliberate: Serial does not invent a global socket/session registry where the Sockets library does not own one.

### ESP-Now

```cpp
#include <ESPressio_ESPNowTransportMonitor.hpp>

ESPressio::Serial::ESPNowTransportMonitor monitor;
monitor.Initialize(Serial);
```

`ESPNowTransportMonitor` consumes the shared `ESPNowTransport` observer surface and reports initialization, shutdown, peer lifecycle, and send acceptance/failure observations.

## Aggregate DiagnosticMonitor

`DiagnosticMonitor` continues to auto-compose integrations that can be located at compile time.

The existing `SystemClock`, `Threads` and `Events` defaults are preserved. 0.5.0 adds:

```cpp
ESPressio::Serial::DiagnosticMonitorConfig config;
config.Commands = true;
config.ESPNow = true;
```

`Commands` and `ESPNow` default to `false` so upgrading Serial does not silently enable additional output even when those optional libraries happen to be present.

Security and Sockets are intentionally not auto-added to the aggregate because they require an explicit runtime instance to observe.

## Command console

`ESPressio_CommandConsole.hpp` remains the transport-neutral Command-backed console integration. Serial 0.5.0 targets the Command 0.3.x generation but does not change the command execution contract.

## Event console

`ESPressio_EventConsole.hpp` remains the opt-in operator-facing Event console for runtime discovery, schema description, JSON composition, validation and dispatch of registered Serializable Events.

The validated Event baseline for 0.5.0 is **Event 5.8.0+ within the 5.x line**. Serializable/ArduinoJson dependencies remain specific to the EventConsole path.

## Event bridges versus Serial monitors

ESPressio Event 5.8.0 can independently convert the same upstream observations into asynchronous Events. Serial monitors and Event bridges are complementary consumers:

```text
              +--> Serial monitor --> Print
Observer -----+
              +--> Event bridge ---> EventManager
```

Serial does not require Event in order to use its Command, Security, Sockets or ESP-Now monitors.

## PlatformIO

Core Serial can still be consumed alone:

```ini
lib_deps =
    flowduino/ESPressio-Serial@^0.5.0
```

Add the ESPressio libraries required by the monitor or console headers selected by the application.

## Compatibility

0.5.0 preserves the existing core Serial, logging, Console, Event Monitor and EventConsole APIs. New monitoring integrations are opt-in. The package metadata intentionally does not make their upstream libraries mandatory.

## License

Apache License 2.0. See [LICENSE](LICENSE).
