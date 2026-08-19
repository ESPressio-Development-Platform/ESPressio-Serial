#pragma once
#include <array>
#include <cstddef>
#include <cstring>
#include <mutex>
#include "ESPressio_ILoggerSink.hpp"

namespace ESPressio::Serial {

template<std::size_t Capacity, std::size_t MessageBytes = 160, std::size_t CategoryBytes = 32>
class DiagnosticRingBuffer final : public ILoggerSink {
public:
    struct StoredEntry {
        uint64_t TimestampMilliseconds = 0;
        LogLevel Level = LogLevel::Info;
        char Category[CategoryBytes]{};
        char Message[MessageBytes]{};
    };

private:
    std::array<StoredEntry, Capacity> _entries{};
    std::size_t _next = 0;
    std::size_t _count = 0;
    mutable std::mutex _mutex;

    static void Copy(char* destination, std::size_t size, const char* source) {
        if (!destination || size == 0) return;
        if (!source) { destination[0] = '\0'; return; }
        std::strncpy(destination, source, size - 1);
        destination[size - 1] = '\0';
    }

public:
    void Write(const LogEntry& entry) override {
        std::lock_guard<std::mutex> lock(_mutex);
        auto& stored = _entries[_next];
        stored.TimestampMilliseconds = entry.TimestampMilliseconds;
        stored.Level = entry.Level;
        Copy(stored.Category, CategoryBytes, entry.Category);
        Copy(stored.Message, MessageBytes, entry.Message);
        _next = (_next + 1) % Capacity;
        if (_count < Capacity) ++_count;
    }

    std::size_t Size() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _count;
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(_mutex);
        _next = _count = 0;
    }

    void Dump(Print& output) const {
        std::lock_guard<std::mutex> lock(_mutex);
        const std::size_t start = (_next + Capacity - _count) % Capacity;
        for (std::size_t i = 0; i < _count; ++i) {
            const auto& e = _entries[(start + i) % Capacity];
            output.print('['); output.print((unsigned long)e.TimestampMilliseconds);
            output.print("] ["); output.print(ToString(e.Level)); output.print("]");
            if (e.Category[0]) { output.print(" ["); output.print(e.Category); output.print(']'); }
            output.print(' '); output.println(e.Message);
        }
    }
};
}
