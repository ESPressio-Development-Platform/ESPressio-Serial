#include <Arduino.h>

#include <ESPressio_ArduinoByteStream.hpp>
#include <ESPressio_SerialLogging.hpp>

inline constexpr auto ApplicationCategory =
    ESPressio::Logging::LogCategory::Named("Application");
inline constexpr auto CameraCategory =
    ESPressio::Logging::LogCategory::Named("Camera");

ESPressio::ESP32Platform::ArduinoByteOutput serialOutput(::Serial);
ESPressio::Serial::SerialLogSink serialSink(serialOutput);

void setup() {
    ::Serial.begin(115200);

    auto& logger = ESPressio::Logging::Logger::GetInstance();
    logger.Router().RegisterSink(&serialSink);

    ESPRESSIO_LOG_INFO(ApplicationCategory, "Boot complete");
    ESPRESSIO_LOG_DEBUG(CameraCategory, "Waiting for connection");
    ESPRESSIO_LOG_WARN(ApplicationCategory, "Example warning");
}

void loop() {}
