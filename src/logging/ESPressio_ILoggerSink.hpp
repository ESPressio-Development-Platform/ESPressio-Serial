#pragma once
#include "../ESPressio_DiagnosticTypes.hpp"
namespace ESPressio::Serial {

/// <summary>Abstract destination for structured ESPressio Serial diagnostic log entries.</summary>
class ILoggerSink {
public:
    virtual ~ILoggerSink() = default;

    /// <summary>Writes one log entry to the sink.</summary>
    /// <param name="entry">Non-owning structured log entry valid for the duration of the call.</param>
    virtual void Write(const LogEntry& entry) = 0;
};
}
