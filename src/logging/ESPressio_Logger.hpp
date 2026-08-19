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

template<std::size_t MaximumSinks = 4>
class Logger {
    std::array<ILoggerSink*, MaximumSinks> _sinks{};
    LogLevel _minimumLevel = LogLevel::Info;
    mutable std::mutex _mutex;

public:
    bool AddSink(ILoggerSink& sink) {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& slot : _sinks) {
            if (slot == &sink) return true;
            if (!slot) { slot = &sink; return true; }
        }
        return false;
    }

    void RemoveSink(ILoggerSink& sink) {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& slot : _sinks) if (slot == &sink) slot = nullptr;
    }

    void SetMinimumLevel(LogLevel level) { _minimumLevel = level; }
    LogLevel GetMinimumLevel() const noexcept { return _minimumLevel; }

    void Log(LogLevel level, const char* category, const char* message) {
        if (level == LogLevel::Off ||
            static_cast<uint8_t>(level) < static_cast<uint8_t>(_minimumLevel) ||
            static_cast<uint8_t>(level) < ESPRESSIO_SERIAL_COMPILETIME_LOG_LEVEL) return;

        LogEntry entry{ static_cast<uint64_t>(millis()), level, category, message };
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto* sink : _sinks) if (sink) sink->Write(entry);
    }

    void Trace(const char* c, const char* m) { Log(LogLevel::Trace,c,m); }
    void Debug(const char* c, const char* m) { Log(LogLevel::Debug,c,m); }
    void Info(const char* c, const char* m) { Log(LogLevel::Info,c,m); }
    void Warning(const char* c, const char* m) { Log(LogLevel::Warning,c,m); }
    void Error(const char* c, const char* m) { Log(LogLevel::Error,c,m); }
    void Critical(const char* c, const char* m) { Log(LogLevel::Critical,c,m); }
};

}
