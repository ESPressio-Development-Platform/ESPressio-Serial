#include <Arduino.h>
#include <ESPressio_Logging.hpp>

ESPressio::Serial::Logger<> logger;
ESPressio::Serial::SerialLogSink serialSink(::Serial);
ESPressio::Serial::DiagnosticRingBuffer<32> history;

void setup() {
    ::Serial.begin(115200);
    logger.AddSink(serialSink);
    logger.AddSink(history);
    logger.SetMinimumLevel(ESPressio::Serial::LogLevel::Debug);

    logger.Info("Application", "Boot complete");
    logger.Debug("Camera", "Waiting for connection");
    logger.Warning("Example", "This warning is retained in the flight recorder");

    ::Serial.println("--- retained history ---");
    history.Dump(::Serial);
}
void loop() {}
