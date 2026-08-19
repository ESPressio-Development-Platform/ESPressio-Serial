# Changelog

All notable changes to this project are documented in this file.

The structure follows the principles of [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and [Semantic Versioning](https://semver.org/).

## [0.2.0] - 2026-08-19

### Added

- Added the ESPressio Serial Diagnostics & Logging Foundation.
- Added `LogLevel`, `LogEntry`, and the pluggable `ILoggerSink` abstraction.
- Added `Logger` with compile-time and runtime severity filtering and multiple simultaneous sinks.
- Added `SerialLogSink` for any Arduino `Print` destination.
- Added fixed-capacity `DiagnosticRingBuffer` for retained in-memory diagnostic history and Serial/`Print` flight-recorder dumps.
- Added opt-in `SystemClockMonitor`, consuming the full ESPressio Timing 2.2.0 System Clock Observer surface including synchronization before/after values and clock differences.
- Added opt-in `ThreadMonitor`, consuming Thread Manager, Thread Garbage Collector, and Thread Termination Dispatcher Observer notifications from ESPressio Threads 3.1.0.
- Added `DiagnosticMonitor`, a convenience aggregator that composes whichever Timing, Threads, and Event monitoring integrations are available to the application.
- Added `ESPressio_Logging.hpp` and `ESPressio_SerialDiagnostics.hpp` batch headers.
- Added Logger, System Clock Monitor, Thread Monitor, and aggregate Diagnostic Monitor examples.
- Added the 0.2.0 feature specification to the repository.

### Changed

- Expanded ESPressio Serial from Event Transport monitoring into a general ESPressio diagnostics and logging layer.
- Updated the core umbrella to expose common diagnostic types while retaining zero mandatory ESPressio-library dependencies.
- Updated package metadata and documentation for version 0.2.0.
- Updated the dependency documentation to describe the new opt-in Timing and Threads relationships.

### Dependency model

- Core ESPressio Serial continues to have no mandatory ESPressio library dependencies.
- Logging requires only the Arduino `Print`/runtime environment.
- System Clock Monitor optionally requires ESPressio Timing 2.2.0 or newer.
- Thread Monitor optionally requires ESPressio Threads 3.1.0 or newer.
- Event Monitor continues to optionally require ESPressio Event 5.5.0 or newer and ESPressio Serializable 0.9.0 or newer.

## [0.1.0] - 2026-08-19

### Added

- Initial ESPressio Serial repository structure and package metadata.
- Added dependency-free `ESPressio_Serial.hpp` core umbrella.
- Added `EventMonitor` as the first opt-in Serial/console integration.
- Added ESPressio Event 5.5.0 Event Transport Transaction Observation support.
- Added monitoring to arbitrary Arduino `Print` destinations.
- Added event-oriented default monitoring and full transaction-lifecycle mode.
- Added metadata-only, summary, hexadecimal, and structured payload output modes.
- Added generic BinaryArchive-to-`SerializationNode` decoding for human-readable payload monitoring without ArduinoJson.
- Added bounded structured-output controls for collection size, string length, nesting depth, and indentation.
- Added Event Monitor configuration for inbound/outbound visibility and transaction metadata fields.
- Added a self-contained loopback Event Transport example demonstrating both outbound and inbound monitoring.

### Dependency model

- Core ESPressio Serial has no mandatory ESPressio library dependencies.
- Event Monitor is opt-in and requires ESPressio Event 5.5.0 or newer plus ESPressio Serializable 0.9.0 or newer.
