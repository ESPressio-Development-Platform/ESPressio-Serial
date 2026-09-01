# Platform Abstractions Audit Trail

This file records Serial changes made during the platform-abstraction and Logging-ownership tranches.

## 2026-09-01 — ESPressio Logging extraction

### Logging ownership
- Removed the Serial-owned `Logger`, `ILoggerSink`, `LogEntry`/level types and diagnostic ring-buffer implementation.
- Added `ESPressio-Logging` as the authoritative owner of Logging records, levels, categories, routing, filtering and Sink contracts.
- Retained only `SerialLogSink` in ESPressio-Serial because translating a Logging record into an operator-facing Serial byte stream is a Serial concern.
- Added the distinct `ESPressio_SerialLogging.hpp` integration header. ESPressio-Serial no longer publishes an `ESPressio_Logging.hpp` header that could shadow the authoritative ESPressio-Logging umbrella.
- Migrated `EventConsole` audit output to the central ESPressio Logger instead of preserving a legacy Serial logging compatibility layer.
- Kept `SerialLogSink` synchronous and non-owning; it does not retain borrowed Logging records or construct an aggregate owning log string.

### Threading and byte output
- `SerialLogSink` still consumes only `System::IO::IByteOutput`.
- Concurrent calls are serialized at the Sink while a complete textual record is emitted so fragments from separate caller threads cannot interleave.
- The Sink's independent Logging level mask is atomic and does not require the output lock for filter checks/configuration.

## 2026-08-27

### Byte I/O
- Added the shared `ESPressio::System::IO` byte-input/output contracts at the System layer.
- Added Arduino `Stream`/`Print` adapters in ESPressio-ESP32.
- Migrated `SerialLogSink` from Arduino `Print` to `System::IO::IByteOutput`.
- Migrated `Console` from Arduino `Stream`/`Print` to `System::IO::IByteInput` and `IByteOutput`, with a convenience overload for one `IByteStream`.
- Preserved CRLF line output, prompt/help formatting, echo, backspace handling, bounded input and line-interceptor semantics.
- Kept console parsing and Serial log rendering in ESPressio-Serial; generic Logging semantics now belong to ESPressio-Logging.

### Command/Event/monitor compatibility correction
- Hardware integration exposed that `CommandConsole`, `EventConsole` and several monitor/formatter surfaces still depended on the historical Arduino `Print` formatting API after `Console::GetOutput()` had moved to the platform-neutral byte-output contract.
- Added `ByteOutputTextWriter`, a Serial-owned formatting facade that implements `System::IO::IByteOutput` while preserving the required `print()`, `println()` and `write()` formatting surface.
- The facade now supports raw byte writes, byte-buffer writes, integral output and floating-point output with explicit precision, matching the monitor formatter operations exercised by the Lab.
- `Console::GetOutput()` exposes that facade. It remains substitutable as `System::IO::IByteOutput*`, while Serial console/monitor layers can consume the Serial-owned text writer without requiring the underlying provider to be Arduino `Print`.
- Preserved command registration/unregistration, line interception, command response routing, Event formatting and monitor payload rendering semantics.
- This correction was discovered by the ESPressio Lab build against the active platform-abstraction branches; the earlier source-wide verification statement was incomplete.

### Verification
- Core `Console` and `SerialLogSink` use only the System byte-I/O abstractions.
- Console, Command/Event console and monitor formatting no longer require the configured output sink itself to be an Arduino `Print` implementation.
- Extended Serial surfaces continue to be audited independently for remaining framework-specific references where platform abstractions apply.

## Boundary rule

System owns generic byte I/O. ESPressio-Logging owns generic Logging semantics and contracts. ESPressio-Serial owns console/operator semantics, Serial-specific log rendering, and diagnostic monitor presentation. Hardware/framework byte-stream types belong in the platform implementation layer.
