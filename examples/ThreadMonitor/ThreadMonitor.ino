#include <Arduino.h>
#include <ESPressio_ThreadMonitor.hpp>
ESPressio::Serial::ThreadMonitor monitor;
void setup() {
    ::Serial.begin(115200);
    monitor.Initialize(::Serial);
}
void loop() { delay(1000); }
