# Changelog

All notable changes to this project are documented in this file.

The structure follows the principles of [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and [Semantic Versioning](https://semver.org/).

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
