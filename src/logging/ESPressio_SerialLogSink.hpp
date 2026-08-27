#pragma once

#include <cstdio>

#include <ESPressio_ByteStream.hpp>

#include "ESPressio_ILoggerSink.hpp"

namespace ESPressio::Serial {

class SerialLogSink final : public ILoggerSink {
    System::IO::IByteOutput* _output;

    static void WriteUnsigned(System::IO::IByteOutput& output, uint64_t value) {
        char buffer[32];
        const int length = std::snprintf(
            buffer,
            sizeof(buffer),
            "%llu",
            static_cast<unsigned long long>(value)
        );
        if (length > 0) {
            output.WriteText(buffer);
        }
    }

public:
    explicit SerialLogSink(System::IO::IByteOutput& output) : _output(&output) {}

    void Write(const LogEntry& entry) override {
        if (!_output) return;

        _output->WriteText("[");
        WriteUnsigned(*_output, entry.TimestampMilliseconds);
        _output->WriteText("] [");
        _output->WriteText(ToString(entry.Level));
        _output->WriteText("]");

        if (entry.Category && *entry.Category) {
            _output->WriteText(" [");
            _output->WriteText(entry.Category);
            _output->WriteText("]");
        }

        _output->WriteText(" ");
        _output->WriteLine(entry.Message ? entry.Message : "");
    }
};

} // namespace ESPressio::Serial
