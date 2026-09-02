# ESPressio Dependency Chart — Serial 0.8.1 working tranche

The existing SVG depicts the last released dependency generation. The text below is authoritative for this working feature tranche while version numbering remains unchanged.

![Released ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.svg)

Arrows point from the consuming library to the library it consumes. Serial remains the terminal/downstream operator layer.

## Current working-branch core

```text
Serial feature/40-logging-sink
    -> System  main
    -> Logging main

Logging main
    -> System      main
    -> Observable  main
    -> Timing      main
```

The new `Logging` edge is required because Serial now provides only the concrete `SerialLogSink`; `Logger`, `LogRouter`, log records, levels, categories and the `ILogSink` abstraction are owned by ESPressio-Logging.

## Released generation retained for reference

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

## Opt-in Serial integrations

These remain downstream/optional and are unchanged by the Logging extraction:

```text
CommandConsole / CommandMonitor
    - - -> Command

SecurityMonitor
    - - -> Security

SocketWorkerMonitor / SocketSecuritySessionMonitor
    - - -> Sockets

ESPNowTransportMonitor
    - - -> ESP-Now

SystemClockMonitor
    - - -> Timing

ThreadMonitor
    - - -> Threads

EventMonitor / EventConsole
    - - -> Event
    - - -> Serializable

WiFiMonitor
    - - -> WiFi
```

`EventConsole` now uses ESPressio-Logging for its audit messages. That does not create an Event→Logging dependency: Serial already consumes Logging as a core dependency, and Event remains unaware of Serial and Logging.

## Dependency-direction invariants

```text
Serial -> Logging
Logging -> Serial   NONE

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

Serial remains terminal/downstream. No upstream ESPressio library should depend on Serial merely to obtain logging; those libraries consume ESPressio-Logging directly.
