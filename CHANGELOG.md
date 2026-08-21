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
- Updated Event Monitor's optional Serializable baseline to ESPressio Serializable >= 0.10.2 < 1.0.0, consuming the bounded/allocation-free BinaryArchive facilities introduced for Flowduino/ESPressio-Serializable#2.

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
