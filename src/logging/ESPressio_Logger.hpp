#pragma once
#include <Arduino.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <mutex>
#include "ESPressio_ILoggerSink.hpp"

#ifndef ESPRESSIO_SERIAL_COMPILETIME_LOG_LEVEL
#define ESPRESSIO_SERIAL_COMPILETIME_LOG_LEVEL 0
#endif

namespace ESPressio::Serial {

/// <summary>Thread-safe fixed-capacity logger that filters diagnostic entries and fans them out to registered sinks.</summary>
/// <typeparam name="MaximumSinks">Maximum number of non-owning logger sinks that may be registered concurrently.</typeparam>
template<std::size_t MaximumSinks = 4>
class Logger {
    std::array<ILoggerSink*, MaximumSinks> _sinks{};
    LogLevel _minimumLevel = LogLevel::Info;
    mutable std::mutex _mutex;

public:
    /// <summary>Adds a non-owning sink if it is not already present.</summary>
    /// <returns>True when the sink is already registered or a free sink slot is available.</returns>
    bool AddSink(ILoggerSink& sink) {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& slot : _sinks) {
            if (slot == &sink) return true;
            if (!slot) { slot = &sink; return true; }
        }
        return false;
    }

    /// <summary>Removes every registration of a sink from the logger.</summary>
    void RemoveSink(ILoggerSink& sink) {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& slot : _sinks) if (slot == &sink) slot = nullptr;
    }

    /// <summary>Sets the runtime minimum severity required for subsequent log entries.</summary>
    void SetMinimumLevel(LogLevel level) { _minimumLevel = level; }
    /// <summary>Returns the runtime minimum severity.</summary>
    LogLevel GetMinimumLevel() const noexcept { return _minimumLevel; }

    /// <summary>Creates and writes one timestamped log entry when it passes runtime and compile-time severity filters.</summary>
    void Log(LogLevel level, const char* category, const char* message) {
        if (level == LogLevel::Off ||
            static_cast<uint8_t>(level) < static_cast<uint8_t>(_minimumLevel) ||
            static_cast<uint8_t>(level) < ESPRESSIO_SERIAL_COMPILETIME_LOG_LEVEL) return;

        LogEntry entry{ static_cast<uint64_t>(millis()), level, category, message };
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto* sink : _sinks) if (sink) sink->Write(entry);
    }

    /// <summary>Logs a Trace-severity message.</summary>
    void Trace(const char* c, const char* m) { Log(LogLevel::Trace,c,m); }
    /// <summary>Logs a Debug-severity message.</summary>
    void Debug(const char* c, const char* m) { Log(LogLevel::Debug,c,m); }
    /// <summary>Logs an Info-severity message.</summary>
    void Info(const char* c, const char* m) { Log(LogLevel::Info,c,m); }
    /// <summary>Logs a Warning-severity message.</summary>
    void Warning(const char* c, const char* m) { Log(LogLevel::Warning,c,m); }
    /// <summary>Logs an Error-severity message.</summary>
    void Error(const char* c, const char* m) { Log(LogLevel::Error,c,m); }
    /// <summary>Logs a Critical-severity message.</summary>
    void Critical(const char* c, const char* m) { Log(LogLevel::Critical,c,m); }
};

}
