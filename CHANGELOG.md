## 0.8.0 — 2026-08-23

### Added

- Added optional `ESPressio_WiFiMonitor.hpp` integration targeting ESPressio WiFi 0.1.0.
- Added `WiFiMonitor` as an `IWiFiObserver` that renders overall mode, independent AP/Client state transitions, scan lifecycle/results, AP station joins/leaves, and Client IP acquisition/loss to an injected Arduino `Print`.
- Added `PrintStatus()` for an on-demand compact WiFi runtime snapshot.
- Added host coverage verifying WiFi lifecycle output and explicitly verifying configured passwords are never emitted by the monitor.
- Added ESP32 compile validation against the WiFi 0.1.0 candidate surface.

### Changed

- Advanced Serial to 0.8.0 because the optional public monitor surface is extended.
- Updated documentation and dependency guidance for the new terminal `Serial - - -> WiFi` integration edge.
- Retained the rule that core Serial has no mandatory ESPressio dependency.

### Security

- `WiFiMonitor` observes runtime WiFi state only; it never reads or prints WiFi password fields.
- Network scan SSIDs and connected Client/AP SSIDs are diagnostic network identity, not credential values.

### Tracking

- Implements #32.

## 0.7.2 — 2026-08-22

### Changed

- Raised the validated optional ESPressio ESP-Now monitor baseline from ESP-Now >= 0.7.0 < 1.0.0 to ESP-Now >= 0.8.0 < 1.0.0.
- Updated ESP32 integration CI to compile Serial's ESP-NOW monitor against released ESP-Now 0.8.0.
- Updated package metadata, README, umbrella dependency guidance, and textual/graphical dependency charts for Serial 0.7.2.
- Retained Command 1.0.0, Security 0.3.0, Sockets 0.7.0, Event 6.0.0, Serializable 0.10.2, Timing 2.2.4, Threads 3.1.4, Units 0.2.3, and Observable 3.0.1 baselines.

### Compatibility

- No Serial public API or runtime semantics are changed.
- Core Serial remains dependency-free.
- ESP-Now remains an opt-in integration.
- ESP-Now 0.8.0 is source-compatible with the previous 0.7.x integration and preserves its wire format and protocol identifiers.

### Tracking

- Implements #26.
- Cascades ESPressio ESP-Now 0.8.0.

## 0.7.1 — 2026-08-22

### Fixed

- `Console::Initialize()` now reserves the configured `MaximumLineLength` capacity before the Console becomes initialized, eliminating an avoidable late `std::string` growth allocation from ordinary `Console::Poll()` input handling.
- Initialization now fails cleanly if the configured bounded input capacity cannot be reserved, rather than leaving a partially initialized Console that may later fail while accepting input.
- Preserved existing maximum-line-length rejection/discard behavior, prompt handling, command dispatch, and public Console API semantics.

### Validation

- Added regression coverage verifying that the configured input capacity is reserved during initialization and retained after normal polling, exact-limit input, line clearing, and over-limit rejection.
- Added regression coverage verifying that an impossible reservation causes `Initialize()` to return `false` while the Console remains uninitialized.

### Compatibility

- No public API changes.
- No dependency baseline changes.
- No downstream ESPressio dependency cascade is required.

### Tracking

- Fixes #10.

## 0.7.0 — 2026-08-22

### Changed

- Raised the validated optional ESPressio Command baseline to Command >= 1.0.0 < 2.0.0.
- Raised the validated optional ESPressio Sockets baseline to Sockets >= 0.7.0 < 1.0.0.
- Raised the validated optional ESPressio ESP-Now baseline to ESP-Now >= 0.7.0 < 1.0.0.
- Updated host validation to released Command 1.0.0.
- Validated the ESP32 monitor integration first against the Sockets 0.7.0 and ESP-Now 0.7.0 cascade candidate heads, then replaced those candidate refs with the actual released 0.7.0 tags and revalidated the final release head before Serial 0.7.0 was merged and released.
- Retained Event 6.0.0, Security 0.3.0, Observable 3.0.1, Serializable 0.10.2, Units 0.2.3, Timing 2.2.4 and Threads 3.1.4 baselines.
- Updated release metadata, README and textual/graphical dependency charts for Serial 0.7.0.

### Architecture

- Core Serial remains dependency-free.
- Command, Sockets, ESP-Now and all other subsystem monitors remain opt-in.
- Serial consumes Command 1.x as the terminal diagnostics/operator layer; it does not introduce a reverse dependency.

### Compatibility

- No Serial public API or runtime semantics are intentionally changed by this cascade.
- Existing Command monitor/console behavior is retained while validating against Command 1.0.0's typed structured-invocation API.

### Tracking

- Implements #20.
- Cascades ESPressio Command 1.0.0 through Sockets 0.7.0 and ESP-Now 0.7.0.

## 0.6.0 — 2026-08-21

### Changed

- Raised the validated optional Event Monitor/EventConsole integration baseline to ESPressio Event >= 6.0.0 < 7.0.0.
- Raised the validated optional Command integration baseline to ESPressio Command >= 0.4.0 < 1.0.0.
- Raised the validated optional Security integration baseline to ESPressio Security >= 0.3.0 < 1.0.0.
- Raised the validated optional Sockets integration baseline to ESPressio Sockets >= 0.6.0 < 1.0.0.
- Raised the validated optional ESP-Now integration baseline to ESPressio ESP-Now >= 0.6.0 < 1.0.0.
- Updated ESP32 integration CI to compile Serial against the fully released architecture generation: Event 6.0.0, Command 0.4.0, Security 0.3.0, Sockets 0.6.0 and ESP-Now 0.6.0.
- Retained Observable 3.0.1, Serializable 0.10.2, Units 0.2.3, Timing 2.2.4 and Threads 3.1.4 baselines.
- Updated release metadata, README and textual/graphical dependency charts for Serial 0.6.0.

### Architecture

- Serial remains the terminal/downstream diagnostics and operator layer.
- Core Serial remains dependency-free.
- All ESPressio integrations remain opt-in.
- Event 6.0.0 no longer owns Command-, Security-, Sockets-, or ESP-Now-specific Event bridges; those integrations are now owned by the corresponding domain libraries, eliminating the previous reciprocal dependency risks upstream of Serial.

### Compatibility

- No Serial public API or runtime semantics are intentionally changed by this dependency cascade.
- Applications selecting Event integrations must use the Event 6.x generation and the corresponding domain-library releases where concrete Event bridge ownership moved.

### Tracking

- Implements #15.

## 0.5.2

### Changed

- Raised the validated optional Event Monitor/EventConsole integration baseline to ESPressio Event >= 5.8.4 < 6.0.0.
- Raised the validated optional ESP-Now transport monitor baseline to ESPressio ESP-Now >= 0.5.3 < 1.0.0.
- Updated ESP32 integration CI to compile Serial against released Event 5.8.4 and ESP-Now 0.5.3.
- Retained the coordinated Serializable 0.10.2, Units 0.2.3, Timing 2.2.4, Threads 3.1.4, Command 0.3.0, Security 0.2.0, Sockets 0.5.0, and Observable 3.0.1 baselines.
- Updated release metadata and dependency documentation for Serial 0.5.2.

### Compatibility

- Core Serial remains dependency-free.
- All Event, ESP-Now, Command, Security, Sockets, Timing, Threads, and Serializable integrations remain opt-in.
- No Serial public API or runtime behavior changes are introduced by this dependency-maintenance release.

## 0.5.1

### Fixed

- Replaced EventMonitor's tree-building `BinaryArchive::Load()` diagnostic path with bounded, allocation-free ESPB traversal from ESPressio Serializable 0.10.2, preventing valid payload diagnostics from requiring a second heap-backed `SerializationNode` tree.
- Hardened structured `EventMonitor` payload diagnostics so malformed, truncated, excessively nested, or otherwise unreasonable Event Transport payloads are rejected under monitor-specific decode limits.
- Added fail-safe fallback from `Structured` to bounded `Hex` output whenever a payload fails structured validation or exceeds the configured monitor limits.
- Updated Event Monitor's optional Serializable baseline to ESPressio Serializable >= 0.10.2 < 1.0.0, consuming the bounded/allocation-free BinaryArchive facilities introduced for ESPressio-Development-Platform/ESPressio-Serializable#2.

### Added

- Added `MaximumStructuredNodes` to `EventMonitorConfig` alongside the existing collection, string, and nesting limits.
- Added deterministic malformed/deep/random payload regression and stress coverage for the EventMonitor structured-payload validation path.
- Added ESP32 compile validation for `EventMonitor` against the coordinated dependency-refresh candidates: Units 0.2.3, Timing 2.2.4, Threads 3.1.4, ESP-Now 0.5.2, Event 5.8.2, and released Serializable 0.10.2.

### Changed

- Raised the current optional Event integration baseline to ESPressio Event >= 5.8.2 < 6.0.0.
- Raised the current optional ESP-Now monitor baseline to ESPressio ESP-Now >= 0.5.2 < 1.0.0.
- Updated current documentation and CI to consume the completed Serializable 0.10.2 dependency cascade rather than intermediate bug-fix commits.
- Serializable 0.10.2 also resolves the strict-build `-Wmisleading-indentation` warning exposed by Serial's warnings-as-errors host validation.

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

## 0.7.3 — 2026-08-22

### Changed
- Published the post-migration ESPressio Serial package generation from `ESPressio-Development-Platform`.
- Raised optional Command to `>=1.0.1 <2.0.0`, Security to `>=0.3.1 <1.0.0`, Sockets to `>=0.7.1 <1.0.0`, ESP-Now to `>=0.8.1 <1.0.0`, Event to `>=6.0.1 <7.0.0`, Serializable to `>=0.10.3 <1.0.0`, Timing to `>=2.2.5 <3.0.0`, and Threads to `>=3.1.5 <4.0.0`.
- Updated package metadata, operator/integration documentation, CI validation, and dependency documentation for the complete migrated release generation.

### Compatibility
- No Serial public API or runtime behaviour changes are introduced by this repository-relocation patch release. Core Serial remains free of mandatory ESPressio dependencies.

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
