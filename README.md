# ESPressio Serial

Serial, console and operator-diagnostics components for the ESPressio Development Platform.

ESPressio Serial is intentionally the **terminal/operator layer** of the ecosystem. It observes and controls other ESPressio subsystems without forcing Serial concerns back into those libraries. Generic logging concepts are owned by `ESPressio-Logging`; this library owns only the concrete Serial Logging Sink.

The canonical branch for the Primitive Platform Redesign is `primitives_redesign`.

## Namespace

```cpp
ESPressio::Serial
```

Because Arduino exposes a global object named `Serial`, fully qualified ESPressio names remain recommended in ESP32 applications.

## Platform-neutral byte I/O

Core Serial does not store Arduino `Stream` or `Print` objects. Console input/output and the Serial Logging Sink consume the portable `ESPressio::System::IO` byte interfaces. Framework-specific byte transport remains in the platform layer.

During coordinated redesign development:

```ini
lib_deps =
    https://github.com/ESPressio-Development-Platform/ESPressio-System.git#primitives_redesign
    https://github.com/ESPressio-Development-Platform/ESPressio-Logging.git#primitives_redesign
    https://github.com/ESPressio-Development-Platform/ESPressio-Serial.git#primitives_redesign
```

On Arduino-ESP32, add ESPressio-ESP32 and create the byte adapter at the application boundary:

```cpp
#include <ESPressio_ArduinoByteStream.hpp>
#include <ESPressio_Console.hpp>

ESPressio::ESP32Platform::ArduinoByteStream consoleIO(::Serial);
ESPressio::Serial::Console console;

void setup() {
    ::Serial.begin(115200);
    console.Initialize(consoleIO);
}
```

## Interactive console

`Console` owns bounded line collection, optional echo, prompt rendering, line interception, command registration and help/error output. It can use one bidirectional byte stream or distinct portable input/output endpoints. The console remains independent of the mechanism carrying those bytes.

### Command tooling

`CommandConsole` is descriptor-driven. It consumes a **frozen `Primitive::TypeDirectoryView`** plus an application-owned `ICommandConsoleAuthorizer`; it owns no Command registry, requester, response route, execution pool, retry path or transport.

Operator flow is therefore:

```text
operator
  -> bounded Serial Console input
  -> frozen TypeDirectory discovery
  -> application authorization
  -> Command P3 schema/constructibility check
  -> Command::SubmitDynamicCommand
  -> normal Command admission/runtime semantics
```

The generic console only submits dynamically constructible fire-and-forget Commands. A response-bearing Command that requires requester capability is reported as unavailable rather than silently manufacturing a requester or bypassing the final response-capability contract.

### Event tooling

`EventConsole` follows the same model. It consumes a frozen `Primitive::TypeDirectoryView`, final Event descriptors and an application-owned `IEventConsoleAuthorizer`. JSON input is bounded by both the Event schema maximum and the configured console maximum before `Event::DispatchDynamicEvent` is invoked.

The console owns no Event registry, listener topology, occurrence pool, retry machinery or transport. Descriptor discovery is not authorization, and a non-constructible Event remains non-constructible even when its descriptor is visible.

### State tooling

Generic State console/monitor surfaces are read/inspect only. State owner-write authority remains with the bound State owner; Serial tooling cannot mutate owner-authoritative State merely because it can discover or diagnose a State Type.

## Logging Sink

`ESPressio-Logging` owns `Logger`, `LogRouter`, record/lease types, levels, categories, metadata and `ILogSink`. ESPressio Serial does not duplicate those concepts.

`SerialLogSink` adapts an ESPressio Logging record to a portable byte output. It executes synchronously on the informing thread, retains no log-record lease, uses bounded stack-local numeric formatting, and serializes complete output records so concurrent callers cannot interleave fragments.

## Diagnostics

Serial provides opt-in downstream diagnostics for the public surfaces of the originating subsystem. Monitors do not invent a parallel runtime model or force diagnostic ownership upstream.

The redesign contracts include:

- `EventMonitor`: final Event descriptor/runtime diagnostics; structured ESPB inspection remains bounded and allocation-safe, with bounded hexadecimal fallback for malformed/out-of-limit payloads.
- Command diagnostics: final Command descriptor/runtime status rather than predecessor registry observation.
- Thread/Timing diagnostics: final `Thread`/capability/resource and Timing quality/evidence seams.
- WiFi diagnostics: native ESPressio-WiFi observer/diagnostic state, never Arduino WiFi internals or plaintext credentials.
- Sockets/Security diagnostics: their respective public contracts only.

## Dynamic-tool security and boundedness

Serial operator tooling follows the same rules as Web and Lua tooling:

- a discoverable Type is not implicitly authorized;
- dynamic input is bounded before parsing or construction;
- P3 schema/factory failure becomes an explicit operator failure;
- no raw reinterpretation or hidden unbounded heap fallback is used to make malformed input succeed;
- Command admission, State ownership and Event delivery/admission policy remain authoritative;
- diagnostics remain downstream and cannot make an upstream library depend on Serial.

## Dependency model

The required dependency surface remains:

```text
Serial
    -> System
    -> Logging
```

Higher-level family integrations are optional downstream consumers selected only when their corresponding headers/features are used. Serial remains terminal/downstream; upstream domain libraries consume their own contracts and must never depend on Serial merely to obtain tooling or Logging.

## Platform boundary

```text
Arduino Serial / Stream / Print
             |
             v
ESPressio-ESP32 byte adapter
             |
             v
System::IO byte contract
             |
             v
ESPressio-Serial Console / SerialLogSink / diagnostics
```

Raw byte transport is generic hardware/runtime I/O and belongs in System. Logging semantics belong in ESPressio-Logging. Serial owns only the operator-facing representation and interaction layer.

## Design principles

- Serial is an operator/diagnostics layer, not a replacement for source-library APIs.
- Core Serial remains framework- and platform-neutral.
- Framework byte-stream types are adapted at the platform/application boundary.
- Optional integrations remain opt-in and downstream.
- Dynamic Command/Event tooling uses frozen TypeDirectory discovery plus final family descriptors/APIs.
- State generic tooling is read-only with respect to owner-authoritative State.
- Authorization is application-owned and independent of descriptor discovery.
- Sensitive configuration values are not emitted merely because diagnostics are enabled.
- Parsing, formatting and dynamic construction limits are bounded for embedded reliability.

## License

Apache License 2.0. See [LICENSE](LICENSE).
