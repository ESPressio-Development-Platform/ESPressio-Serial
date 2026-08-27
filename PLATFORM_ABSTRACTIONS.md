# Platform Abstractions Audit Trail

This file records Serial changes made during the platform-abstraction tranche tracked by issue #43.

## 2026-08-27

### Byte I/O
- Added the shared `ESPressio::System::IO` byte-input/output contracts at the System layer.
- Added Arduino `Stream`/`Print` adapters in ESPressio-ESP32.
- Migrated `SerialLogSink` from Arduino `Print` to `System::IO::IByteOutput`.
- Migrated `Console` from Arduino `Stream`/`Print` to `System::IO::IByteInput` and `IByteOutput`, with a convenience overload for one `IByteStream`.
- Preserved CRLF line output, prompt/help formatting, echo, backspace handling, bounded input and line-interceptor semantics.
- Kept console parsing and log formatting in ESPressio-Serial; the platform abstraction transports bytes only.

### Verification
- Source-wide searches show no remaining `Arduino.h` or `freertos/*` includes in the repository.
- Core Serial is therefore framework/platform neutral; target-specific stream adaptation belongs in the top-level platform composition layer.

## Boundary rule

Serial owns framing, parsing, console, logging and monitoring semantics. Hardware/framework byte-stream types belong in the platform implementation layer.
