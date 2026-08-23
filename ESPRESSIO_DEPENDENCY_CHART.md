# ESPressio Dependency Chart — Serial 0.8.0 Candidate

![ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.svg)

Arrows point from the consuming library to the library it consumes. Serial remains the terminal/downstream operator layer.

## Coordinated candidate/released generation

```text
Observable    3.0.2
Serializable  0.11.0   candidate merged, release pending
Units         0.2.4
Timing        2.2.5
Threads       3.1.5
Event         6.0.1
Command       1.0.1
Security      0.4.0    candidate merged, release pending
Persistence   0.3.0    candidate merged, release pending
Sockets       0.7.1
ESP-Now       0.8.1
WiFi          0.1.0    candidate
Serial        0.8.0    candidate
```

## Serial core

```text
Serial core 0.8.0
    -> none
```

## Opt-in Serial integrations

```text
CommandConsole / CommandMonitor
    - - -> Command >= 1.0.1 < 2.0.0

SecurityMonitor
    - - -> Security >= 0.4.0 < 1.0.0

SocketWorkerMonitor / SocketSecuritySessionMonitor
    - - -> Sockets >= 0.7.1 < 1.0.0

ESPNowTransportMonitor
    - - -> ESP-Now >= 0.8.1 < 1.0.0

SystemClockMonitor
    - - -> Timing >= 2.2.5 < 3.0.0

ThreadMonitor
    - - -> Threads >= 3.1.5 < 4.0.0

EventMonitor / EventConsole
    - - -> Event >= 6.0.1 < 7.0.0
    - - -> Serializable >= 0.11.0 < 1.0.0

WiFiMonitor
    - - -> WiFi >= 0.1.0 < 1.0.0
```

WiFi itself requires Observable and Serializable and optionally consumes Persistence/Security/Event/Command. Those remain WiFi-owned edges; Serial does not duplicate them simply because it can observe WiFi.

```text
WiFi 0.1.0
    -> Observable >= 3.0.2 < 4.0.0
    -> Serializable >= 0.11.0 < 1.0.0
    - - -> Persistence >= 0.3.0 < 1.0.0
    - - -> Security >= 0.4.0 < 1.0.0
    - - -> Event >= 6.0.1 < 7.0.0
    - - -> Command >= 1.0.1 < 2.0.0
```

## Dependency-direction invariants

```text
Serial - - -> WiFi
WiFi -> Serial   NONE

Serial - - -> Security
Security -> Serial   NONE

Serial - - -> Event
Event -> Serial   NONE
```

Domain-specific Event and Command adapters remain in their owning libraries. Web is higher-order and is not introduced into either WiFi or Serial by this feature.
