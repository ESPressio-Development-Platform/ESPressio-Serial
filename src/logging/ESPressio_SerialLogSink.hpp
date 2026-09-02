#pragma once

#include <atomic>
#include <cstdio>
#include <mutex>
#include <string_view>
#include <type_traits>
#include <variant>

#include <ESPressio_ByteStream.hpp>
#include <ESPressio_ILogSink.hpp>
#include <ESPressio_LogField.hpp>
#include <ESPressio_LogLevel.hpp>

namespace ESPressio::Serial {

/// <summary>Synchronously renders ESPressio structured log records to a byte-output destination.</summary>
/// <remarks>The Sink owns no output buffer, never retains the supplied LogRecordLease, and serializes complete records so concurrent callers cannot interleave one another's output fragments.</remarks>
class SerialLogSink final : public Logging::ILogSink {
    System::IO::IByteOutput* _output = nullptr;
    std::atomic<Logging::LogLevelMask> _levelMask{Logging::AllLogLevels};
    mutable std::mutex _writeMutex;

    static void WriteView(System::IO::IByteOutput& output, std::string_view value) noexcept {
        if (value.empty()) return;
        std::size_t written = 0;
        (void)output.Write(
            reinterpret_cast<const uint8_t*>(value.data()),
            value.size(),
            written
        );
    }

    static void WriteUnsigned(System::IO::IByteOutput& output, uint64_t value) noexcept {
        char buffer[32];
        const int length = std::snprintf(
            buffer,
            sizeof(buffer),
            "%llu",
            static_cast<unsigned long long>(value)
        );
        if (length > 0) WriteView(output, std::string_view(buffer, static_cast<std::size_t>(length)));
    }

    static void WriteSigned(System::IO::IByteOutput& output, int64_t value) noexcept {
        char buffer[32];
        const int length = std::snprintf(
            buffer,
            sizeof(buffer),
            "%lld",
            static_cast<long long>(value)
        );
        if (length > 0) WriteView(output, std::string_view(buffer, static_cast<std::size_t>(length)));
    }

    static void WriteFloating(System::IO::IByteOutput& output, double value) noexcept {
        char buffer[40];
        const int length = std::snprintf(buffer, sizeof(buffer), "%.9g", value);
        if (length > 0) WriteView(output, std::string_view(buffer, static_cast<std::size_t>(length)));
    }

    static const char* LevelName(Logging::LogLevel level) noexcept {
        switch (level) {
            case Logging::LogLevel::Trace: return "TRACE";
            case Logging::LogLevel::Debug: return "DEBUG";
            case Logging::LogLevel::Info: return "INFO";
            case Logging::LogLevel::Warn: return "WARN";
            case Logging::LogLevel::Error: return "ERROR";
            case Logging::LogLevel::Fatal: return "FATAL";
        }
        return "?";
    }

    static void WriteFieldValue(System::IO::IByteOutput& output, const Logging::LogFieldValue& value) noexcept {
        std::visit(
            [&output](const auto& typedValue) noexcept {
                using TValue = std::decay_t<decltype(typedValue)>;
                if constexpr (std::is_same_v<TValue, bool>) {
                    WriteView(output, typedValue ? "true" : "false");
                } else if constexpr (std::is_same_v<TValue, int32_t> || std::is_same_v<TValue, int64_t>) {
                    WriteSigned(output, static_cast<int64_t>(typedValue));
                } else if constexpr (std::is_same_v<TValue, uint32_t> || std::is_same_v<TValue, uint64_t>) {
                    WriteUnsigned(output, static_cast<uint64_t>(typedValue));
                } else if constexpr (std::is_same_v<TValue, float> || std::is_same_v<TValue, double>) {
                    WriteFloating(output, static_cast<double>(typedValue));
                } else if constexpr (std::is_same_v<TValue, std::string_view>) {
                    WriteView(output, typedValue);
                }
            },
            value
        );
    }

public:
    /// <summary>Creates a synchronous Serial logging Sink over a non-owning byte-output destination.</summary>
    explicit SerialLogSink(
        System::IO::IByteOutput& output,
        Logging::LogLevelMask levelMask = Logging::AllLogLevels
    ) noexcept : _output(&output), _levelMask(levelMask) {}

    /// <summary>Changes the levels accepted by this Sink without blocking an in-progress record write.</summary>
    void SetLevelMask(Logging::LogLevelMask levelMask) noexcept {
        _levelMask.store(levelMask, std::memory_order_relaxed);
    }

    /// <summary>Returns the levels currently accepted by this Sink.</summary>
    Logging::LogLevelMask GetLevelMask() const noexcept {
        return _levelMask.load(std::memory_order_relaxed);
    }

    /// <inheritdoc/>
    bool IsEnabled(Logging::LogLevel level, const Logging::LogCategory&) const noexcept override {
        return _output != nullptr && Logging::ContainsLevel(GetLevelMask(), level);
    }

    /// <inheritdoc/>
    void Accept(const Logging::LogRecordLease& record) noexcept override {
        if (_output == nullptr) return;
        std::lock_guard<std::mutex> writeLock(_writeMutex);

        const auto& view = record.View();

        WriteView(*_output, "[mono=");
        WriteUnsigned(*_output, view.Timestamp.MonotonicNanoseconds);
        WriteView(*_output, "ns");
        if (view.Timestamp.SystemNanoseconds != 0) {
            WriteView(*_output, " system=");
            WriteUnsigned(*_output, view.Timestamp.SystemNanoseconds);
            WriteView(*_output, "ns");
        }
        WriteView(*_output, "] [");
        WriteView(*_output, LevelName(view.Level));
        WriteView(*_output, "]");

        if (!view.Category.Name.empty()) {
            WriteView(*_output, " [");
            WriteView(*_output, view.Category.Name);
            WriteView(*_output, "]");
        }

        WriteView(*_output, " ");
        WriteView(*_output, view.Message);

        for (const auto& field : view.Metadata) {
            WriteView(*_output, " ");
            WriteView(*_output, field.Name);
            WriteView(*_output, "=");
            WriteFieldValue(*_output, field.Value);
        }

        (void)_output->WriteLine();
    }
};

} // namespace ESPressio::Serial
