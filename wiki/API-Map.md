# API Map

This page provides conceptual navigation for the principal ESPressio Serial surfaces.

## Portable operator I/O

- `Console` — bounded interactive line-oriented console.
- System `IByteInput` / `IByteOutput` / `IByteStream` — upstream portable byte contracts consumed by Serial.

## Logging

- `Logger<>` — record distribution to configured sinks.
- `SerialLogSink` — formatted output to portable byte output.
- `DiagnosticRingBuffer<N>` — bounded retained diagnostic history.

## Domain operator integrations

- `CommandConsole` — Console to authoritative Command registry/handlers.
- `EventConsole` — discovery/composition/dispatch of registered Serializable Events.

## Monitors

Optional monitors adapt public observer surfaces from Timing, Threads, Event, Command, Security, Sockets, ESP-Now, WiFi and other supported domains into concise diagnostics.

## Dependency direction

```text
Serial core -> System

Serial optional integration - - -> domain library
```

No upstream domain library should depend on Serial.

## Platform adapters

Target-specific adapters such as Arduino byte-stream wrappers belong to the target/platform package (for ESP32, ESPressio-ESP32), not to reusable Serial core.

Use the dedicated Wiki pages for behavioural and extension contracts rather than treating this map as exhaustive generated API documentation.