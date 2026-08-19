#pragma once
#include <cstdint>

namespace ESPressio::Serial {

enum class LogLevel : uint8_t {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Critical = 5,
    Off = 255
};

inline const char* ToString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        case LogLevel::Off: return "OFF";
    }
    return "?";
}

struct LogEntry {
    uint64_t TimestampMilliseconds = 0;
    LogLevel Level = LogLevel::Info;
    const char* Category = nullptr;
    const char* Message = nullptr;
};

}
