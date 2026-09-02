# ESPressio Serial 1.0.0

ESPressio Serial is the terminal, operator and diagnostics layer of the ESPressio Development Platform.

It provides portable console handling, logging, bounded diagnostic history and opt-in monitors for other ESPressio subsystems without forcing Serial concerns back into those upstream libraries.

## Architectural boundary

```text
platform/framework byte I/O
          |
          v
ESPressio-System IO contracts
          |
          v
ESPressio-Serial
 console / logging / diagnostics
          |
          +--> optional Command console
          +--> optional Event console/monitor
          +--> optional subsystem monitors
```

Serial core consumes `System::IO::IByteInput`, `IByteOutput` and `IByteStream`. Framework-specific objects such as Arduino `Stream`/`Print` are adapted by the target/platform layer rather than stored in Serial core.

## Start here

- [Getting Started](Getting-Started)
- [Portable Byte IO](Portable-Byte-IO)
- [Console](Console)
- [Logging and Diagnostic History](Logging-and-Diagnostics)
- [Command and Event Consoles](Command-and-Event-Consoles)
- [Subsystem Monitors](Subsystem-Monitors)
- [Security and Diagnostic Safety](Diagnostic-Safety)
- [Memory and Performance](Memory-and-Performance)
- [Extending Serial](Extending-Serial)
- [API Map](API-Map)

## Dependency rule

Serial is deliberately downstream. The core requires System for portable byte I/O; integrations with Command, Security, Timing, Threads, Event, WiFi, ESP-Now, Sockets and other domains are optional and must not cause those libraries to depend back on Serial.

## Version baseline

This Wiki documents the intended ESPressio **1.0.0** platform baseline and therefore describes the current platform-neutral architecture rather than preserving historical pre-1.0 release numbering.