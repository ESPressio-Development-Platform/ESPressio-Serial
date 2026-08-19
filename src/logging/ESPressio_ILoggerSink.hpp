#pragma once
#include "../ESPressio_DiagnosticTypes.hpp"
namespace ESPressio::Serial {
class ILoggerSink {
public:
    virtual ~ILoggerSink() = default;
    virtual void Write(const LogEntry& entry) = 0;
};
}
