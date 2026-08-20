## 0.5.1

### Fixed

- Replaced EventMonitor's tree-building `BinaryArchive::Load()` diagnostic path with bounded, allocation-free ESPB traversal from ESPressio Serializable 0.10.1, preventing valid payload diagnostics from requiring a second heap-backed `SerializationNode` tree.
- Hardened structured `EventMonitor` payload diagnostics so malformed, truncated, excessively nested, or otherwise unreasonable Event Transport payloads are rejected under monitor-specific decode limits.
- Added fail-safe fallback from `Structured` to bounded `Hex` output whenever a payload fails structured validation or exceeds the configured monitor limits.
- Updated Event Monitor's optional Serializable baseline to ESPressio Serializable >= 0.10.1 < 1.0.0, consuming the bounded/allocation-free BinaryArchive facilities introduced for Flowduino/ESPressio-Serializable#2.

### Added

- Added `MaximumStructuredNodes` to `EventMonitorConfig` alongside the existing collection, string, and nesting limits.
- Added deterministic malformed/deep/random payload regression and stress coverage for the EventMonitor structured-payload validation path.
- Added ESP32 compile validation for `EventMonitor` against the coordinated dependency-refresh candidates: Units 0.2.2, Timing 2.2.3, Threads 3.1.3, ESP-Now 0.5.1, Event 5.8.1, and released Serializable 0.10.1.

### Changed

- Raised the current optional Event integration baseline to ESPressio Event >= 5.8.1 < 6.0.0.
- Raised the current optional ESP-Now monitor baseline to ESPressio ESP-Now >= 0.5.1 < 1.0.0.
- Updated current documentation and CI to consume the completed Serializable 0.10.1 dependency cascade rather than intermediate bug-fix commits.

### Compatibility

- Core Serial remains dependency-free.
- EventMonitor remains opt-in.
- Existing structured output remains JSON-like and source-compatible for payloads that validate successfully.

## 0.5.0

- Added opt-in `CommandMonitor` for ESPressio Command 0.3.x registry lifecycle observation.
- Added opt-in `SecurityMonitor` for ESPressio Security 0.2.x configuration, session, replay and failure observation.
- Added opt-in `SocketWorkerMonitor` and `SocketSecuritySessionMonitor` for ESPressio Sockets 0.5.x lifecycle observation.
- Added opt-in `ESPNowTransportMonitor` for ESPressio ESP-Now 0.5.x transport, peer and send lifecycle observation.
- Extended `DiagnosticMonitor` with optional Command and ESP-Now monitoring while preserving its existing default behavior.
- Updated the validated optional ESPressio Event integration baseline to Event 5.8.0 within the 5.x line.
- Preserved Serial's dependency-free core: all new monitors are selected explicitly and observe the originating subsystem rather than duplicating its lifecycle semantics.

## 0.4.0

- Added optional ESPressio Command 0.2.x integration through `CommandConsole`.
- Added a Command-backed EventConsole initialization path and registered `event`/`events` command trees.
- Preserved the legacy Console registration path for backward compatibility.
- Added scoped Command registration cleanup during EventConsole shutdown.
- Updated integration guidance for the current ESPressio dependency generation.

# Changelog

## 0.3.3 — 2026-08-20

### Changed
- Updated the validated optional ESPressio Event integration baseline from Event 5.6.2 to Event 5.7.1 within the 5.x line.
- Updated referenced Timing and Threads baselines to Timing 2.2.2 and Threads 3.1.2, matching the cascaded dependency-refresh release chain.
- Updated the validated Serializable baseline to 0.10.0 for Event Monitor/EventConsole integrations that consume Serializable facilities.
- Updated package metadata for Serial 0.3.3.
- Core Serial, Console, logging, and diagnostics remain dependency-free unless the corresponding ESPressio integration is explicitly selected.
- No Serial public interfaces or runtime semantics changed.

## 0.3.2 — 2026-08-19

### Changed
- Updated active ESPressio dependency baselines to the latest released versions available on 2026-08-19.
- Bounded dependency compatibility to the current major version so future breaking major releases are not selected automatically.
- Updated optional ESPressio Event integrations to require Event 5.6.2 or newer within the 5.x line.
- Updated referenced Timing and Threads integration baselines to Timing 2.2.1 and Threads 3.1.1, matching the dependency-refresh release chain.

All notable changes to this project are documented in this file.

The structure follows the principles of [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and [Semantic Versioning](https://semver.org/).

## [0.3.1] - 2026-08-19

### Changed

- Updated the optional `EventConsole` integration baseline from ESPressio Event 5.6.0 to **ESPressio Event 5.6.1**.
- Updated EventConsole compile-time dependency guidance and PlatformIO examples to target Event 5.6.1.
- Updated ESPressio Serial package/component version metadata to 0.3.1.

### Fixed

- Removed the upstream full-stack validation blocker caused by Event 5.6.0 lacking `EventDispatchContext` equality semantics for ESPressio Threads 3.1 `ReadWriteMutex<T>` change detection; the correction is supplied by Event 5.6.1.

### Compatibility

- No ESPressio Serial public interfaces or runtime behaviour are changed by this patch release.
- The generic `Console`, logging, diagnostic monitors, and Event Monitor remain source-compatible.
- `EventConsole` remains opt-in and now validates against ESPressio Event 5.6.1 or newer.

## [0.3.0] - 2026-08-19

### Added

- Added the **Interactive Runtime Serializable Event Console** feature.
- Added the generic `Console` foundation using Arduino `Stream` input and `Print` output.
- Added bounded line-oriented console input, configurable prompt/echo behaviour, command registration/unregistration, argument handling, and built-in help.
- Added multiple simultaneous interactive line interceptors so independently implemented console extensions can coexist.
- Added the opt-in `EventConsole` integration for ESPressio Event 5.6.0.
- Added runtime listing of registered Serializable Event types.
- Added runtime Event schema/metadata description using Event 5.6 `SerializableEventDescriptor` snapshots.
- Added JSON-to-`SerializationNode` Event composition through ESPressio Serializable's optional `JsonArchive`.
- Added detailed Serializable validation/deserialization issue reporting.
- Added ownership-safe runtime Queue and Stack dispatch through Event 5.6.
- Added one-line Event dispatch commands and interactive JSON composition mode.
- Added optional pre-dispatch operator confirmation.
- Added safe-by-default Event dispatch authorization using allow-list, allow-all, and deny-list policies; deny-list entries override broader access.
- Added optional `ILoggerSink` audit integration for operator dispatch, denial, malformed JSON, and construction/dispatch failures.
- Added `Console`, `EventConsole`, and `EventConsoleLoopback` examples.
- Added host-side `Console` tests and an `EventConsole` contract test covering discovery, description, access policy, JSON command handling, confirmation, and dispatch.
- Added the 0.3.0 feature specification.

### Changed

- Expanded ESPressio Serial from diagnostics/output tooling into an interactive operator/service-console layer.
- Updated package metadata and build include paths for version 0.3.0.
- Updated dependency documentation for ESPressio Event 5.6.0 and the optional Serializable JSON adapter.
- Preserved the dependency-free core and all 0.1/0.2 diagnostics/logging interfaces.

### Dependency model

- Core ESPressio Serial continues to have no mandatory ESPressio library dependencies.
- Generic `Console` requires no additional ESPressio dependency.
- `EventConsole` is opt-in and requires ESPressio Event 5.6.0 or newer, ESPressio Serializable 0.9.0 or newer, and the optional ArduinoJson dependency used by `JsonArchive`.
- Existing System Clock, Threads, Event Monitor, and logging integrations remain independently opt-in.

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