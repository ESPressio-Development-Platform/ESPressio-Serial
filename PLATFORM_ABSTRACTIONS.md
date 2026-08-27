# Platform Abstractions Audit Trail

This file records Serial changes made during the platform-abstraction tranche tracked by issue #43.

## 2026-08-27

### Byte output
- Added the shared `ESPressio::System::IO` byte-input/output contracts at the System layer.
- Added Arduino `Stream`/`Print` adapters in ESPressio-ESP32.
- Migrated `SerialLogSink` from Arduino `Print` to `System::IO::IByteOutput`.
- Kept log formatting in ESPressio-Serial; the platform abstraction transports bytes only.

### Remaining work
- `Console` still directly uses Arduino `Stream` and `Print` and is the next byte-stream consumer to migrate.
- After Console migration, perform a source-wide check for any remaining Arduino/ESP32 byte-stream types and update README examples to construct the ESPressio-ESP32 adapter at the application boundary.

## Boundary rule

Serial owns framing, parsing, console, logging and monitoring semantics. Hardware/framework byte-stream types belong in the platform implementation layer.
