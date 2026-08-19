#pragma once
#include <Arduino.h>
#include "ESPressio_ILoggerSink.hpp"

namespace ESPressio::Serial {
class SerialLogSink final : public ILoggerSink {
    Print* _output;
public:
    explicit SerialLogSink(Print& output) : _output(&output) {}
    void Write(const LogEntry& entry) override {
        if (!_output) return;
        _output->print('['); _output->print((unsigned long)entry.TimestampMilliseconds);
        _output->print("] ["); _output->print(ToString(entry.Level)); _output->print(']');
        if (entry.Category && *entry.Category) {
            _output->print(" ["); _output->print(entry.Category); _output->print(']');
        }
        _output->print(' ');
        _output->println(entry.Message ? entry.Message : "");
    }
};
}
