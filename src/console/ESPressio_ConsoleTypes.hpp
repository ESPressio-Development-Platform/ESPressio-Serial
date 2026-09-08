#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>

#include <ESPressio_Memory.hpp>

namespace ESPressio::Serial {

/// <summary>Outcome from parsing and executing one console input line.</summary>
/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
enum
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
class ConsoleExecutionResult : uint8_t {
    Executed,
    Empty,
    UnknownCommand,
    LineTooLong
};

/// <summary>Configures console line buffering, input echo, and prompt presentation.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - MaximumLineLength (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * - EchoInput (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowPrompt (bool): 1 bytes [0 bytes dynamic allocation]
 * - Prompt (System::Memory::String<System::Memory::MemoryPolicy::ExternalPreferred>): sizeof(System::Memory::String<System::Memory::MemoryPolicy::ExternalPreferred>) [0 bytes dynamic allocation]
 * Total Memory: 6 bytes known members + sizeof(System::Memory::String<System::Memory::MemoryPolicy::ExternalPreferred>) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
struct ConsoleConfig {
    /// <summary>Maximum accepted input line length before characters are discarded until newline.</summary>
    std::size_t MaximumLineLength = 2048;
    /// <summary>Echo received input characters to the output stream.</summary>
    bool EchoInput = false;
    /// <summary>Write the configured prompt after initialization and each completed line.</summary>
    bool ShowPrompt = true;
    /// <summary>Prompt text written when ShowPrompt is enabled; retained storage prefers external memory.</summary>
    System::Memory::String<System::Memory::MemoryPolicy::ExternalPreferred> Prompt = "> ";
};

/// <summary>Non-owning command and argument views supplied to a registered console command handler.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - Command (std::string_view): sizeof(std::string_view) [0 bytes dynamic allocation]
 * - Arguments (std::string_view): sizeof(std::string_view) [0 bytes dynamic allocation]
 * Total Memory: sizeof(std::string_view) + sizeof(std::string_view) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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