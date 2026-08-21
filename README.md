# ESPressio Serial

Serial and console-oriented components for the Flowduino ESPressio Development Platform.

Version 0.5.2 completes the Event / ESP-Now reliability dependency cascade by validating the opt-in Event Monitor and EventConsole integrations against ESPressio Event 5.8.4 and the ESP-Now transport monitor against ESPressio ESP-Now 0.5.3. It retains the EventMonitor structured-payload hardening introduced in 0.5.1 and preserves the dependency-free Serial core.

## Current Version — 0.5.2

Version **0.5.2** is a dependency-maintenance release. No Serial public API or runtime semantics change.

Current optional integration baselines are:

```text
CommandConsole / CommandMonitor
    - - -> ESPressio Command >= 0.3.0 < 1.0.0

SecurityMonitor
    - - -> ESPressio Security >= 0.2.0 < 1.0.0

SocketWorkerMonitor
    - - -> ESPressio Sockets >= 0.5.0 < 1.0.0

SocketSecuritySessionMonitor
    - - -> ESPressio Sockets >= 0.5.0 < 1.0.0
    - - -> ESPressio Security >= 0.2.0 < 1.0.0

ESPNowTransportMonitor
    - - -> ESPressio ESP-Now >= 0.5.3 < 1.0.0

SystemClockMonitor
    - - -> ESPressio Timing >= 2.2.4 < 3.0.0

ThreadMonitor
    - - -> ESPressio Threads >= 3.1.4 < 4.0.0

EventMonitor / EventConsole
    - - -> ESPressio Event >= 5.8.4 < 6.0.0
    - - -> ESPressio Serializable >= 0.10.2 < 1.0.0
```

`DiagnosticMonitor` can additionally compose `CommandMonitor` and `ESPNowTransportMonitor` when those dependencies are present. Security and Socket monitors remain instance-oriented because the application must choose the specific `TransportSecurity`, `SocketWorker`, or `SocketSecuritySession` object to observe.

These monitors subscribe directly to the originating library's Observable contract. They do not invent parallel Serial lifecycle semantics and do not require ESPressio Event. Event-backed observation remains a separate opt-in integration in ESPressio Event 5.8.4.

Serial 0.5.2 retains the structured EventMonitor safety introduced in 0.5.1: bounded, allocation-free ESPB traversal from ESPressio Serializable 0.10.2 with fail-safe hexadecimal fallback for malformed or outside-limit payloads.

Current coordinated dependency baselines are Observable 3.0.1, Serializable 0.10.2, Units 0.2.3, Timing 2.2.4, Threads 3.1.4, ESP-Now 0.5.3, Event 5.8.4, Command 0.3.0, Security 0.2.0, and Sockets 0.5.0.

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
ESPressio Event >= 5.8.4 < 6.0.0
ESPressio Serializable >= 0.10.2 < 1.0.0
```

The additional opt-in monitoring dependencies for the 0.5.x line are listed above.

For the complete ecosystem hierarchy, see:

**[ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.md)**

In the dependency chart:

- **Solid relationships** represent required dependencies.
- **Dashed relationships** represent opt-in dependencies introduced only when the associated feature/header is used.

---

## Version 0.5.1 — Structured EventMonitor safety

Version 0.5.1 fixed the structured EventMonitor crash path reproduced on ESP32 under low-memory conditions. Structured diagnostics use ESPressio Serializable 0.10.2's allocation-free BinaryArchive traversal API with explicit depth, aggregate-node, collection, name, and string limits. Invalid or outside-limit payloads fall back to bounded hexadecimal output rather than becoming fatal diagnostic work.

Historical release details remain available in [CHANGELOG.md](CHANGELOG.md).
