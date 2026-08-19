#include <Arduino.h>
#include <ESPressio_DiagnosticMonitor.hpp>
ESPressio::Serial::DiagnosticMonitor diagnostics;
void setup() {
    ::Serial.begin(115200);
    ESPressio::Serial::DiagnosticMonitorConfig config;
    diagnostics.Initialize(::Serial, config);
}
void loop() { delay(1000); }
