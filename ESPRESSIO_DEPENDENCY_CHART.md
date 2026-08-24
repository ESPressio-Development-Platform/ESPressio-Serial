# ESPressio Dependency Chart — Serial 0.8.1

![ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.svg)

Arrows point from the consuming library to the library it consumes. Serial remains the terminal/downstream operator layer.

## Released generation

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

## Serial core

```text
Serial core 0.8.1
    -> none
```

## Opt-in Serial integrations

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

WiFi owns its own required/optional dependency edges; Serial does not duplicate them merely because it observes WiFi.

```text
WiFi 0.2.0
    -> Observable >= 3.0.2 < 4.0.0
    -> Serializable >= 0.11.3 < 1.0.0
    -> Threads >= 3.1.7 < 4.0.0
    - - -> Persistence >= 0.3.2 < 1.0.0
    - - -> Security >= 0.4.2 < 1.0.0
    - - -> Event >= 6.0.3 < 7.0.0
    - - -> Command >= 1.0.3 < 2.0.0
```

## Dependency-direction invariants

```text
Serial - - -> WiFi
WiFi -> Serial   NONE

Serial - - -> Security
Security -> Serial   NONE

Serial - - -> Event
Event -> Serial   NONE

Serial - - -> Sockets
Sockets -> Serial   NONE

Serial - - -> ESP-Now
ESP-Now -> Serial   NONE
```

Serial is terminal/downstream. No upstream ESPressio library should depend on Serial. Domain-specific Event and Command adapters remain in their owning libraries.
