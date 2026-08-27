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

### Command/Event console compatibility correction
- Hardware integration exposed that `CommandConsole` and `EventConsole` still retained historical `Print*` output members after `Console::GetOutput()` had moved to the platform-neutral byte-output contract.
- Added `ByteOutputTextWriter`, a Serial-owned formatting facade that implements `System::IO::IByteOutput` while preserving the existing `print()` / `println()` console formatting surface.
- `Console::GetOutput()` now exposes that facade. It remains substitutable as `System::IO::IByteOutput*`, while the existing Command/Event console layers can consume the Serial-owned text writer without requiring the underlying output provider to be an Arduino `Print` object.
- Preserved command registration/unregistration, line interception, command response routing and Event console formatting semantics.
- This correction was discovered by the ESPressio Lab build against the active platform-abstraction branches; the earlier verification statement was therefore incomplete.

### Verification
- Core `Console` and `SerialLogSink` use only the System byte-I/O abstractions.
- Command/Event console output no longer requires the configured output sink itself to be an Arduino `Print` implementation.
- Remaining Arduino-specific includes/usages in extended Serial surfaces must not be treated as evidence that the configured byte-output provider is Arduino-specific; they should continue to be audited independently where platform abstractions apply.

## Boundary rule

Serial owns framing, parsing, console, logging and monitoring semantics. Hardware/framework byte-stream types belong in the platform implementation layer.
