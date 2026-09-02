# Portable Byte IO

Serial core consumes the platform-neutral byte contracts owned by ESPressio System:

```cpp
ESPressio::System::IO::IByteInput
ESPressio::System::IO::IByteOutput
ESPressio::System::IO::IByteStream
```

## Why this boundary exists

A console or logger needs bytes, but it should not care whether those bytes travel through Arduino UART, USB CDC, a host test stream or another target-specific mechanism.

Framework classes such as Arduino `Stream` and `Print` therefore do not belong in the reusable Serial core API.

```text
Arduino/SDK/native endpoint
          |
          v
 target adapter/provider
          |
          v
 System::IO contract
          |
          v
 Serial semantics
```

## Bidirectional and split endpoints

Use `IByteStream` where one object provides both directions. Use independent `IByteInput` and `IByteOutput` implementations when input and output use different endpoints.

## Testing advantage

Because Serial depends on byte interfaces, console parsing and logging can be exercised with deterministic in-memory/test endpoints without requiring a UART or Arduino runtime.

## Extension boundary

Do not add a framework-specific stream abstraction to Serial merely to support a new platform. Add/adapt the raw byte mechanism in the appropriate platform/provider layer and satisfy the System IO contract.

Serial extensions should deal with the **meaning** of bytes to an operator or diagnostic consumer, not with the device driver that moves them.