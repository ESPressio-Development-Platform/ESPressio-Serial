#pragma once

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <ESPressio_ByteStream.hpp>

#include "ESPressio_ConsoleTypes.hpp"

namespace ESPressio::Serial {

// Text-formatting facade over the platform-neutral byte-output contract.
//
// Console-oriented layers historically consumed Arduino Print directly. This
// writer keeps their convenient print/println formatting semantics inside the
// Serial domain while the actual output remains a System::IO::IByteOutput.
class ByteOutputTextWriter final : public System::IO::IByteOutput {
private:
    System::IO::IByteOutput* _output = nullptr;

    template<typename TValue>
    void PrintIntegral(TValue value) noexcept {
        if (_output == nullptr) return;

        char buffer[32];
        if constexpr (std::is_signed_v<TValue>) {
            std::snprintf(
                buffer,
                sizeof(buffer),
                "%lld",
                static_cast<long long>(value)
            );
        } else {
            std::snprintf(
                buffer,
                sizeof(buffer),
                "%llu",
                static_cast<unsigned long long>(value)
            );
        }
        (void)_output->WriteText(buffer);
    }

    template<typename TValue>
    void PrintFloating(TValue value) noexcept {
        if (_output == nullptr) return;

        char buffer[48];
        std::snprintf(
            buffer,
            sizeof(buffer),
            "%.6g",
            static_cast<double>(value)
        );
        (void)_output->WriteText(buffer);
    }

public:
    ByteOutputTextWriter() = default;

    explicit ByteOutputTextWriter(System::IO::IByteOutput* output) noexcept
        : _output(output) {}

    void Bind(System::IO::IByteOutput* output) noexcept {
        _output = output;
    }

    System::IO::IByteOutput* GetOutput() const noexcept {
        return _output;
    }

    System::PlatformResult Write(
        const uint8_t* data,
        std::size_t size,
        std::size_t& bytesWritten
    ) noexcept override {
        bytesWritten = 0;
        if (_output == nullptr) {
            return System::PlatformResult::Failed(
                System::PlatformStatus::Unavailable
            );
        }
        return _output->Write(data, size, bytesWritten);
    }

    void print(const char* text) noexcept {
        if (_output != nullptr && text != nullptr) {
            (void)_output->WriteText(text);
        }
    }

    void print(const std::string& text) noexcept {
        print(text.c_str());
    }

    void print(char value) noexcept {
        if (_output != nullptr) {
            (void)_output->WriteByte(static_cast<uint8_t>(value));
        }
    }

    template<
        typename TValue,
        std::enable_if_t<
            std::is_integral_v<TValue> &&
            !std::is_same_v<std::remove_cv_t<TValue>, char>,
            int
        > = 0
    >
    void print(TValue value) noexcept {
        PrintIntegral(value);
    }

    template<
        typename TValue,
        std::enable_if_t<std::is_floating_point_v<TValue>, int> = 0
    >
    void print(TValue value) noexcept {
        PrintFloating(value);
    }

    void println() noexcept {
        if (_output != nullptr) {
            (void)_output->WriteLine();
        }
    }

    void println(const char* text) noexcept {
        if (_output != nullptr) {
            (void)_output->WriteLine(text);
        }
    }

    void println(const std::string& text) noexcept {
        println(text.c_str());
    }

    void println(char value) noexcept {
        print(value);
        println();
    }

    template<
        typename TValue,
        std::enable_if_t<
            std::is_integral_v<TValue> &&
            !std::is_same_v<std::remove_cv_t<TValue>, char>,
            int
        > = 0
    >
    void println(TValue value) noexcept {
        print(value);
        println();
    }

    template<
        typename TValue,
        std::enable_if_t<std::is_floating_point_v<TValue>, int> = 0
    >
    void println(TValue value) noexcept {
        print(value);
        println();
    }
};

// Transitional source-compatibility name for the Serial console extensions
// that historically stored Arduino Print*. Because this alias lives in the
// ESPressio::Serial namespace, it does not alter or expose the Arduino type.
using Print = ByteOutputTextWriter;

class Console final {
private:
    struct CommandRegistration {
        std::string Name;
        std::string Help;
        ConsoleCommandHandler Handler;
    };

    System::IO::IByteInput* _input = nullptr;
    System::IO::IByteOutput* _output = nullptr;
    mutable ByteOutputTextWriter _textOutput;
    ConsoleConfig _config;

    std::vector<CommandRegistration> _commands;

    std::string _line;
    bool _discardUntilNewline = false;

    struct InterceptorRegistration {
        uint32_t ID = 0;
        ConsoleLineInterceptor Handler;
    };

    std::vector<InterceptorRegistration> _interceptors;
    uint32_t _nextInterceptorID = 1;

    static std::string_view Trim(std::string_view value) noexcept {
        while (
            !value.empty() &&
            std::isspace(static_cast<unsigned char>(value.front()))
        ) {
            value.remove_prefix(1);
        }

        while (
            !value.empty() &&
            std::isspace(static_cast<unsigned char>(value.back()))
        ) {
            value.remove_suffix(1);
        }

        return value;
    }

    static bool EqualsIgnoreCase(
        std::string_view left,
        std::string_view right
    ) noexcept {
        if (left.size() != right.size()) return false;

        for (std::size_t index = 0; index < left.size(); ++index) {
            if (
                std::tolower(static_cast<unsigned char>(left[index])) !=
                std::tolower(static_cast<unsigned char>(right[index]))
            ) {
                return false;
            }
        }

        return true;
    }

    void WriteText(const char* text) {
        if (_output != nullptr && text != nullptr) {
            (void)_output->WriteText(text);
        }
    }

    void WriteLine(const char* text = nullptr) {
        if (_output != nullptr) {
            (void)_output->WriteLine(text);
        }
    }

    void PrintPrompt() {
        if (_output != nullptr && _config.ShowPrompt) {
            WriteText(_config.Prompt.c_str());
        }
    }

    void PrintHelp(std::string_view command = {}) {
        if (_output == nullptr) return;

        command = Trim(command);

        if (!command.empty()) {
            for (const auto& registration : _commands) {
                if (EqualsIgnoreCase(registration.Name, command)) {
                    WriteText(registration.Name.c_str());
                    if (!registration.Help.empty()) {
                        WriteText(" - ");
                        WriteLine(registration.Help.c_str());
                    } else {
                        WriteLine();
                    }
                    return;
                }
            }

            WriteText("Unknown command: ");
            const std::string value(command);
            WriteLine(value.c_str());
            return;
        }

        WriteLine("Available commands:");
        WriteLine("  help [command]");

        for (const auto& registration : _commands) {
            WriteText("  ");
            WriteText(registration.Name.c_str());
            if (!registration.Help.empty()) {
                WriteText(" - ");
                WriteText(registration.Help.c_str());
            }
            WriteLine();
        }
    }

public:
    Console() = default;

    bool Initialize(
        System::IO::IByteInput& input,
        System::IO::IByteOutput& output,
        const ConsoleConfig& config = {}
    ) {
        ConsoleConfig preparedConfig;
        std::string preparedLine;

        try {
            preparedConfig = config;
            preparedLine.reserve(preparedConfig.MaximumLineLength);
        } catch (...) {
            return false;
        }

        _input = &input;
        _output = &output;
        _textOutput.Bind(_output);
        _config = std::move(preparedConfig);
        _line = std::move(preparedLine);
        _discardUntilNewline = false;

        PrintPrompt();
        return true;
    }

    bool Initialize(
        System::IO::IByteStream& stream,
        const ConsoleConfig& config = {}
    ) {
        return Initialize(stream, stream, config);
    }

    void Shutdown() {
        _input = nullptr;
        _output = nullptr;
        _textOutput.Bind(nullptr);
        _line.clear();
        _discardUntilNewline = false;
        _interceptors.clear();
    }

    bool GetIsInitialized() const noexcept {
        return _input != nullptr && _output != nullptr;
    }

    System::IO::IByteInput* GetInput() const noexcept {
        return _input;
    }

    ByteOutputTextWriter* GetOutput() const noexcept {
        return _output == nullptr ? nullptr : &_textOutput;
    }

#ifdef ESPRESSIO_SERIAL_TESTING
    std::size_t __GetInputBufferCapacityForTesting() const noexcept {
        return _line.capacity();
    }
#endif

    uint32_t RegisterLineInterceptor(ConsoleLineInterceptor interceptor) {
        if (!interceptor) return 0;

        const uint32_t id = _nextInterceptorID++;
        _interceptors.push_back({id, std::move(interceptor)});
        return id;
    }

    bool UnregisterLineInterceptor(uint32_t id) {
        const auto found = std::remove_if(
            _interceptors.begin(),
            _interceptors.end(),
            [&](const auto& item) { return item.ID == id; }
        );

        if (found == _interceptors.end()) return false;
        _interceptors.erase(found, _interceptors.end());
        return true;
    }

    bool RegisterCommand(
        std::string name,
        std::string help,
        ConsoleCommandHandler handler
    ) {
        if (name.empty() || !handler) return false;

        for (const auto& registration : _commands) {
            if (EqualsIgnoreCase(registration.Name, name)) return false;
        }

        _commands.push_back({
            std::move(name),
            std::move(help),
            std::move(handler)
        });

        std::sort(
            _commands.begin(),
            _commands.end(),
            [](const auto& left, const auto& right) {
                return left.Name < right.Name;
            }
        );

        return true;
    }

    bool UnregisterCommand(std::string_view name) {
        const auto found = std::remove_if(
            _interceptors.begin(),
            _interceptors.end(),
            [&](const auto& item) {
                return EqualsIgnoreCase(item.Name, name);
            }
        );

        if (found == _interceptors.end()) return false;
        _interceptors.erase(found, _interceptors.end());
        return true;
    }

    ConsoleExecutionResult ExecuteLine(std::string_view line) {
        if (_output == nullptr) {
            return ConsoleExecutionResult::UnknownCommand;
        }

        line = Trim(line);
        if (line.empty()) return ConsoleExecutionResult::Empty;

        for (const auto& interceptor : _interceptors) {
            if (interceptor.Handler && interceptor.Handler(line)) {
                return ConsoleExecutionResult::Executed;
            }
        }

        const auto separator = line.find_first_of(" \t");
        const auto command =
            separator == std::string_view::npos
                ? line
                : line.substr(0, separator);
        const auto arguments =
            separator == std::string_view::npos
                ? std::string_view{}
                : Trim(line.substr(separator + 1));

        if (EqualsIgnoreCase(command, "help")) {
            PrintHelp(arguments);
            return ConsoleExecutionResult::Executed;
        }

        for (const auto& registration : _commands) {
            if (!EqualsIgnoreCase(registration.Name, command)) continue;

            registration.Handler({command, arguments});
            return ConsoleExecutionResult::Executed;
        }

        WriteText("Unknown command: ");
        const std::string commandText(command);
        WriteLine(commandText.c_str());
        WriteLine("Type 'help' for available commands.");

        return ConsoleExecutionResult::UnknownCommand;
    }

    void Poll() {
        if (_input == nullptr || _output == nullptr) return;

        while (_input->Available() > 0) {
            uint8_t value = 0;
            if (!_input->Read(value)) break;

            const char character = static_cast<char>(value);

            if (character == '\r') continue;

            if (character == '\n') {
                if (_discardUntilNewline) {
                    _discardUntilNewline = false;
                    _line.clear();
                    WriteLine("Input rejected: line exceeds configured maximum length.");
                    PrintPrompt();
                    continue;
                }

                if (_config.EchoInput) WriteLine();
                ExecuteLine(_line);
                _line.clear();
                PrintPrompt();
                continue;
            }

            if (character == '\b' || character == 0x7F) {
                if (!_line.empty()) {
                    _line.pop_back();
                    if (_config.EchoInput) WriteText("\b \b");
                }
                continue;
            }

            if (_discardUntilNewline) continue;

            if (_line.size() >= _config.MaximumLineLength) {
                _discardUntilNewline = true;
                continue;
            }

            _line.push_back(character);

            if (_config.EchoInput) {
                (void)_output->WriteByte(static_cast<uint8_t>(character));
            }
        }
    }
};

} // namespace ESPressio::Serial
