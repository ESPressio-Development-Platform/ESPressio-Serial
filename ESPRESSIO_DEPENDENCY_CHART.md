# ESPressio Dependency Chart

![ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.png)

## Purpose

This document describes the current dependency relationships between ESPressio libraries.

The chart is hierarchical: libraries with no **required** ESPressio dependencies appear at the top, while libraries that build on progressively more of the ecosystem appear lower.

- **Solid arrow** — required ESPressio dependency.
- **Dashed arrow** — opt-in dependency activated only by the associated feature/header.
- Arrows point from the dependent library to the library it consumes.

## ESPressio Serial

The ESPressio Serial core and generic Console have no required ESPressio dependency.

Its diagnostics integrations are opt-in. `EventMonitor` consumes:

```text
ESPressio Event >= 5.6.0
ESPressio Serializable >= 0.9.0
```

Event supplies the Event Transport Transaction Observation stream.

Serializable supplies `BinaryArchive` decoding into the generic `SerializationNode` tree used for human-readable structured payload output.

Applications using only the core ESPressio Serial layer do not acquire either dependency.

`SystemClockMonitor` optionally consumes ESPressio Timing 2.2.0 or newer, while `ThreadMonitor` optionally consumes ESPressio Threads 3.1.0 or newer. These relationships are also dashed/opt-in.

## Other relationships

- Timing requires Units and Observable.
- Threads requires Timing and Observable.
- ESP-Now requires Timing and optionally Event for ESP-NOW Event Transport.
- Event requires Threads, Timing, and Observable; Serializable functionality remains opt-in.
- Sockets has no mandatory ESPressio dependency but optionally consumes Event for network Event Transports and Timing for socket/SNTP clock synchronization.
- Units optionally consumes Serializable for Serializable Unit counterparts.


## ESPressio Serial Event Console

`EventConsole` adds no new mandatory core relationship.

When explicitly selected it consumes:

```text
ESPressio Event >= 5.6.0
    runtime Serializable Event discovery, descriptors, construction and dispatch

ESPressio Serializable >= 0.9.0
    JsonArchive and validation diagnostics
```

Both remain opt-in relationships.

The external ArduinoJson dependency is required only by the optional Serializable `JsonArchive`; it is outside this ESPressio-to-ESPressio dependency chart.
