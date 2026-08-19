# ESPressio Dependency Chart

![ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.png)

## Purpose

This document describes the current dependency relationships between ESPressio libraries.

The chart is hierarchical: libraries with no **required** ESPressio dependencies appear at the top, while libraries that build on progressively more of the ecosystem appear lower.

- **Solid arrow** — required ESPressio dependency.
- **Dashed arrow** — opt-in dependency activated only by the associated feature/header.
- Arrows point from the dependent library to the library it consumes.

## ESPressio Serial

The ESPressio Serial core has no required ESPressio dependency.

Its first integration, `EventMonitor`, is opt-in and consumes:

```text
ESPressio Event >= 5.5.0
ESPressio Serializable >= 0.9.0
```

Event supplies the Event Transport Transaction Observation stream.

Serializable supplies `BinaryArchive` decoding into the generic `SerializationNode` tree used for human-readable structured payload output.

Applications using only the core ESPressio Serial layer do not acquire either dependency.

## Other relationships

- Timing requires Units and Observable.
- Threads requires Timing and Observable.
- ESP-Now requires Timing and optionally Event for ESP-NOW Event Transport.
- Event requires Threads, Timing, and Observable; Serializable functionality remains opt-in.
- Sockets has no mandatory ESPressio dependency but optionally consumes Event for network Event Transports and Timing for socket/SNTP clock synchronization.
- Units optionally consumes Serializable for Serializable Unit counterparts.
