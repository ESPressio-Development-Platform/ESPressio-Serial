# Getting Started

ESPressio Serial core works with portable System byte-I/O interfaces rather than framework-specific stream classes.

## ESP32 composition

On Arduino-ESP32, adapt the framework stream at the application/platform boundary and pass the portable adapter to Serial:

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

The global Arduino object is also named `Serial`, so fully qualified ESPressio names are recommended in ESP32 applications.

## Separate endpoints

Input and output may be supplied separately:

```cpp
console.Initialize(input, output);
```

Any implementation satisfying the System byte contracts can be used: UART adapters, USB serial, host/test streams or another target provider.

## What Serial owns

Serial interprets bytes as operator/diagnostic interactions. It owns line collection, prompts, logging formats, monitor output and optional command/event operator surfaces.

It does **not** own the hardware/runtime mechanism that transports individual bytes.

## Next steps

Read [Portable Byte IO](Portable-Byte-IO) for the platform boundary, then [Console](Console) or [Logging and Diagnostic History](Logging-and-Diagnostics) according to the required operator surface.