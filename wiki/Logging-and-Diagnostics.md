# Logging and Diagnostic History

The logging layer separates log records from their destinations.

A typical composition is:

```cpp
#include <ESPressio_Logging.hpp>
#include <ESPressio_ArduinoByteStream.hpp>

ESPressio::ESP32Platform::ArduinoByteOutput serialOutput(::Serial);
ESPressio::Serial::Logger<> logger;
ESPressio::Serial::SerialLogSink serialSink(serialOutput);
ESPressio::Serial::DiagnosticRingBuffer<64> history;

logger.AddSink(serialSink);
logger.AddSink(history);
logger.Info("Application", "Boot complete");
```

## `SerialLogSink`

`SerialLogSink` writes compact timestamp/level/category formatted records to a portable `IByteOutput`. It does not require Arduino `Print` in the logging implementation.

## Multiple sinks

A logger can feed more than one sink. This permits immediate operator output and a bounded diagnostic history to coexist without coupling the producer to either destination.

## `DiagnosticRingBuffer`

The diagnostic history is bounded. When selecting its capacity, account for the memory cost of retained records and the amount of history genuinely useful for fault analysis.

A bounded ring is preferred to an ever-growing diagnostic log on embedded targets.

## Logging discipline

Keep high-frequency paths concise. Avoid expensive formatting or large payload dumps in timing-sensitive callbacks unless diagnostic mode explicitly requires them.

Never assume logging is free: byte output can be slow, and formatting/history storage consume CPU and memory even when the application is otherwise healthy.