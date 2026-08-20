# ESPressio Dependency Chart

![ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.png)

## Purpose

This document describes the current dependency relationships between ESPressio libraries.

The chart is hierarchical: libraries with no **required** ESPressio dependencies appear at the top, while libraries that build on progressively more of the ecosystem appear lower.

- **Solid arrow** — required ESPressio dependency.
- **Dashed arrow** — opt-in dependency activated only by the associated feature/header.
- Arrows point from the dependent library to the library it consumes.

## ESPressio Serial 0.4.0

The ESPressio Serial core and generic `Console` have no required ESPressio dependency.

All integrations remain opt-in.

### CommandConsole

`CommandConsole` consumes:

```text
ESPressio Command >= 0.2.0 < 1.0.0
```

Command supplies the transport-neutral typed Command registry, parsing, validation, invocation, help/completion metadata, and ownership-safe scoped command registration used by dynamic Serial integrations.

### EventMonitor

`EventMonitor` consumes:

```text
ESPressio Event >= 5.7.1 < 6.0.0
ESPressio Serializable >= 0.10.0 < 1.0.0
```

Event supplies the Event Transport Transaction Observation stream.

Serializable supplies structured payload decoding used for human-readable diagnostic output.

### Timing and Threads monitors

`SystemClockMonitor` optionally consumes:

```text
ESPressio Timing >= 2.2.2 < 3.0.0
```

`ThreadMonitor` optionally consumes:

```text
ESPressio Threads >= 3.1.2 < 4.0.0
```

## ESPressio Serial Event Console

The legacy EventConsole initialization path remains supported for compatibility.

The recommended Serial 0.4.0 Command-backed EventConsole integration consumes:

```text
ESPressio Command >= 0.2.0 < 1.0.0
ESPressio Event >= 5.7.1 < 6.0.0
ESPressio Serializable >= 0.10.0 < 1.0.0
```

Command supplies the shared `event`/`events` command tree and scoped registration lifetime.

Event supplies runtime Serializable Event discovery, descriptors, construction and dispatch.

Serializable supplies `JsonArchive` and validation diagnostics.

The external ArduinoJson dependency is required only by the optional Serializable `JsonArchive`; it is outside this ESPressio-to-ESPressio dependency chart.

Applications using only the core ESPressio Serial layer acquire none of these optional ESPressio dependencies.

## Other current relationships

- Observable 3.0.1 has no mandatory ESPressio dependencies.
- Serializable 0.10.0 has no mandatory ESPressio dependencies.
- Units 0.2.1 optionally consumes Serializable for Serializable Unit counterparts.
- Timing 2.2.2 requires Units and Observable.
- Threads 3.1.2 requires Timing and Observable.
- Event 5.7.1 requires Threads, Timing, and Observable; Serializable functionality remains opt-in.
- ESP-Now 0.2.3 requires Timing and optionally Event for ESP-NOW Event Transport.
- Sockets 0.2.3 has no mandatory ESPressio dependency but optionally consumes Event and Timing.
- Command 0.2.0 has no mandatory ESPressio dependencies.
- Serial 0.4.0 has no mandatory ESPressio dependencies; Command/Event/Serializable/Timing/Threads integrations are opt-in.
