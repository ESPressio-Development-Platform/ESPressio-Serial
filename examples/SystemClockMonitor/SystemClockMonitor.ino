#include <Arduino.h>
#include <ESPressio_SystemClockMonitor.hpp>
ESPressio::Serial::SystemClockMonitor<> monitor;
void setup() {
    ::Serial.begin(115200);
    monitor.Initialize(::Serial);
}
void loop() { delay(1000); }
