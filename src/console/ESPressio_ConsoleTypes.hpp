#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

namespace ESPressio::Serial {

enum class ConsoleExecutionResult : uint8_t {
    Executed,
    Empty,
    UnknownCommand,
    LineTooLong
};

struct ConsoleConfig {
    std::size_t MaximumLineLength = 2048;
    bool EchoInput = false;
    bool ShowPrompt = true;
    std::string Prompt = "> ";
};

struct ConsoleCommandContext {
    std::string_view Command;
    std::string_view Arguments;
};

using ConsoleCommandHandler =
    std::function<void(
        const ConsoleCommandContext&
    )>;

using ConsoleLineInterceptor =
    std::function<bool(
        std::string_view
    )>;

}
