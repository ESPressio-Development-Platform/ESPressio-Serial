#pragma once
#include <cstdint>

namespace ESPressio::Serial {

/// <summary>Severity level assigned to a diagnostic log entry.</summary>
enum class LogLevel : uint8_t {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Critical = 5,
    Off = 255
};

/// <summary>Returns the stable uppercase display name for a diagnostic log level.</summary>
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

/// <summary>Non-owning diagnostic log record supplied to Serial logging sinks.</summary>
struct LogEntry {
    /// <summary>Timestamp associated with the entry, expressed in milliseconds.</summary>
    uint64_t TimestampMilliseconds = 0;
    /// <summary>Severity level of the entry.</summary>
    LogLevel Level = LogLevel::Info;
    /// <summary>Optional non-owning category string.</summary>
    const char* Category = nullptr;
    /// <summary>Optional non-owning message string.</summary>
    const char* Message = nullptr;
};

}
