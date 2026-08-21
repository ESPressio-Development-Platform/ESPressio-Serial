# ESPressio Serial

Serial and console-oriented components for the Flowduino ESPressio Development Platform.

## Current Version — 0.6.0

Version **0.6.0** completes the downstream cascade for the Event 6.0.0 dependency-architecture correction. Serial remains the terminal diagnostics/operator layer: its core has no mandatory ESPressio dependencies, while integrations are selected explicitly.

Current optional integration baselines are:

```text
CommandConsole / CommandMonitor
    - - -> ESPressio Command >= 0.4.0 < 1.0.0

SecurityMonitor
    - - -> ESPressio Security >= 0.3.0 < 1.0.0

SocketWorkerMonitor
    - - -> ESPressio Sockets >= 0.6.0 < 1.0.0

SocketSecuritySessionMonitor
    - - -> ESPressio Sockets >= 0.6.0 < 1.0.0
    - - -> ESPressio Security >= 0.3.0 < 1.0.0

ESPNowTransportMonitor
    - - -> ESPressio ESP-Now >= 0.6.0 < 1.0.0

SystemClockMonitor
    - - -> ESPressio Timing >= 2.2.4 < 3.0.0

ThreadMonitor
    - - -> ESPressio Threads >= 3.1.4 < 4.0.0

EventMonitor / EventConsole
    - - -> ESPressio Event >= 6.0.0 < 7.0.0
    - - -> ESPressio Serializable >= 0.10.2 < 1.0.0
```

The coordinated released generation is:

```text
Observable    3.0.1
Serializable  0.10.2
Units         0.2.3
Timing        2.2.4
Threads       3.1.4
Command       0.4.0
Security      0.3.0
Event         6.0.0
Sockets       0.6.0
ESP-Now       0.6.0
Serial        0.6.0
```

Event 6.0.0 now contains only the generic Event mechanism plus integrations for libraries Event genuinely consumes itself. Command-, Security-, Sockets-, and ESP-Now-specific Event integrations are supplied by their owning libraries, eliminating the previous reciprocal dependency risks. Serial consumes those libraries only through its explicit monitor/console integrations and remains downstream of the entire graph.

`DiagnosticMonitor` can compose selected monitors when their dependencies are present. Security and Socket monitors remain instance-oriented because the application chooses the concrete `TransportSecurity`, `SocketWorker`, or `SocketSecuritySession` object to observe.

Serial 0.6.0 retains the structured EventMonitor safety introduced in 0.5.1: bounded, allocation-free ESPB traversal from ESPressio Serializable 0.10.2 with fail-safe hexadecimal fallback for malformed or outside-limit payloads.

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

For the complete ecosystem hierarchy, see:

**[ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.md)**

In the dependency chart:

- **Solid relationships** represent required dependencies.
- **Dashed relationships** represent opt-in dependencies introduced only when the associated feature/header is used.

Historical release details are available in [CHANGELOG.md](CHANGELOG.md).
