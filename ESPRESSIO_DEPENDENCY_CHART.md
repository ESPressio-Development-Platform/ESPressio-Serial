# ESPressio Dependency Chart

![ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.png)

## Purpose

This document describes the current dependency relationships between ESPressio libraries relevant to ESPressio Serial 0.5.0.

The chart is hierarchical: libraries with no **required** ESPressio dependencies appear at the top, while libraries that build on progressively more of the ecosystem appear lower.

- **Solid arrow** — required ESPressio dependency.
- **Dashed arrow** — opt-in dependency activated only by the associated feature/header.
- Arrows point from the dependent library to the library it consumes.

## ESPressio Serial 0.5.0

The ESPressio Serial core and generic `Console` have no required ESPressio dependency.

All integrations remain opt-in.

### CommandConsole and CommandMonitor

`CommandConsole` and `CommandMonitor` consume:

```text
ESPressio Command >= 0.3.0 < 1.0.0
```

Command supplies the transport-neutral typed Command registry, parsing, validation, invocation, help/completion metadata, scoped command registration, and Observable registry lifecycle used by Serial's Command integrations.

### SecurityMonitor

`SecurityMonitor` consumes:

```text
ESPressio Security >= 0.2.0 < 1.0.0
```

Security supplies the Observable configuration, secure-session, replay-protection, and failure lifecycle observed directly by the monitor.

### Socket monitors

`SocketWorkerMonitor` consumes:

```text
ESPressio Sockets >= 0.5.0 < 1.0.0
```

`SocketSecuritySessionMonitor` consumes:

```text
ESPressio Sockets >= 0.5.0 < 1.0.0
ESPressio Security >= 0.2.0 < 1.0.0
```

Sockets supplies the Observable socket worker and secure-session lifecycle contracts. Security is only relevant to the secure-session integration.

### ESPNowTransportMonitor

`ESPNowTransportMonitor` consumes:

```text
ESPressio ESP-Now >= 0.5.0 < 1.0.0
```

ESP-Now supplies the Observable transport, peer, and send lifecycle contract.

### EventMonitor and EventConsole

`EventMonitor` consumes:

```text
ESPressio Event >= 5.8.0 < 6.0.0
ESPressio Serializable >= 0.10.0 < 1.0.0
```

Event supplies the Event Transport Transaction Observation stream. Serializable supplies structured payload decoding used for human-readable diagnostic output.

The legacy EventConsole initialization path remains supported for compatibility. The recommended Command-backed EventConsole integration consumes:

```text
ESPressio Command >= 0.3.0 < 1.0.0
ESPressio Event >= 5.8.0 < 6.0.0
ESPressio Serializable >= 0.10.0 < 1.0.0
```

Command supplies the shared `event`/`events` command tree and scoped registration lifetime. Event supplies runtime Serializable Event discovery, descriptors, construction and dispatch. Serializable supplies `JsonArchive` and validation diagnostics.

The external ArduinoJson dependency is required only by the optional Serializable `JsonArchive`; it is outside this ESPressio-to-ESPressio dependency chart.

### Timing and Threads monitors

`SystemClockMonitor` optionally consumes:

```text
ESPressio Timing >= 2.2.2 < 3.0.0
```

`ThreadMonitor` optionally consumes:

```text
ESPressio Threads >= 3.1.2 < 4.0.0
```

### Event bridges versus Serial monitors

The 0.5.0 Observable monitors subscribe directly to the originating subsystem. They do not require ESPressio Event.

ESPressio Event 5.8.0 separately supplies optional Event bridges for Command, Security, Sockets, and ESP-Now when asynchronous Event conversion is desired. Serial diagnostics therefore remain usable without introducing Event as an intermediary.

## Current ecosystem relationships

- Observable 3.0.1 has no mandatory ESPressio dependencies.
- Serializable 0.10.0 has no mandatory ESPressio dependencies.
- Units 0.2.1 optionally consumes Serializable for Serializable Unit counterparts.
- Timing 2.2.2 requires Units and Observable.
- Threads 3.1.2 requires Timing and Observable.
- Security 0.2.0 requires Observable; Event conversion is opt-in downstream through Event 5.8.0.
- Command 0.3.0 requires Observable; Event conversion is opt-in downstream through Event 5.8.0.
- Sockets 0.5.0 consumes Observable for lifecycle observation and optionally integrates Command and Security.
- ESP-Now 0.5.0 requires Timing and Observable and optionally integrates Command, Security, and Event transport functionality.
- Event 5.8.0 requires Threads, Timing, and Observable and optionally bridges Security, Command, Sockets, and ESP-Now observer contracts.
- Serial 0.5.0 has no mandatory ESPressio dependencies; Command, Security, Sockets, ESP-Now, Event, Serializable, Timing, and Threads integrations are all opt-in.

Applications using only the core ESPressio Serial layer acquire none of these optional ESPressio dependencies.
