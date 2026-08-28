#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

namespace ESPressio::Serial {

/// <summary>Outcome from parsing and executing one console input line.</summary>
enum class ConsoleExecutionResult : uint8_t {
    Executed,
    Empty,
    UnknownCommand,
    LineTooLong
};

/// <summary>Configures console line buffering, input echo, and prompt presentation.</summary>
struct ConsoleConfig {
    /// <summary>Maximum accepted input line length before characters are discarded until newline.</summary>
    std::size_t MaximumLineLength = 2048;
    /// <summary>Echo received input characters to the output stream.</summary>
    bool EchoInput = false;
    /// <summary>Write the configured prompt after initialization and each completed line.</summary>
    bool ShowPrompt = true;
    /// <summary>Prompt text written when ShowPrompt is enabled.</summary>
    std::string Prompt = "> ";
};

/// <summary>Non-owning command and argument views supplied to a registered console command handler.</summary>
struct ConsoleCommandContext {
    std::string_view Command;
    std::string_view Arguments;
};

/// <summary>Callback invoked for a named console command.</summary>
using ConsoleCommandHandler =
    std::function<void(
        const ConsoleCommandContext&
    )>;

/// <summary>Pre-command callback that may consume a complete console line before normal command lookup.</summary>
/// <returns>True when the interceptor consumed the line and normal command execution should stop.</returns>
using ConsoleLineInterceptor =
    std::function<bool(
        std::string_view
    )>;

}
