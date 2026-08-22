#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <Arduino.h>

#include "ESPressio_ConsoleTypes.hpp"

namespace ESPressio::Serial {

class Console final {
private:
    struct CommandRegistration {
        std::string Name;
        std::string Help;
        ConsoleCommandHandler Handler;
    };

    Stream* _input = nullptr;
    Print* _output = nullptr;
    ConsoleConfig _config;

    std::vector<CommandRegistration>
        _commands;

    std::string _line;
    bool _discardUntilNewline = false;
    struct InterceptorRegistration {
        uint32_t ID = 0;
        ConsoleLineInterceptor Handler;
    };

    std::vector<InterceptorRegistration>
        _interceptors;

    uint32_t _nextInterceptorID = 1;

    static std::string_view Trim(
        std::string_view value
    ) noexcept {
        while (
            !value.empty() &&
            std::isspace(
                static_cast<unsigned char>(
                    value.front()
                )
            )
        ) {
            value.remove_prefix(1);
        }

        while (
            !value.empty() &&
            std::isspace(
                static_cast<unsigned char>(
                    value.back()
                )
            )
        ) {
            value.remove_suffix(1);
        }

        return value;
    }

    static bool EqualsIgnoreCase(
        std::string_view left,
        std::string_view right
    ) noexcept {
        if (left.size() != right.size()) {
            return false;
        }

        for (
            std::size_t index = 0;
            index < left.size();
            ++index
        ) {
            if (
                std::tolower(
                    static_cast<unsigned char>(
                        left[index]
                    )
                ) !=
                std::tolower(
                    static_cast<unsigned char>(
                        right[index]
                    )
                )
            ) {
                return false;
            }
        }

        return true;
    }

    void PrintPrompt() {
        if (
            _output != nullptr &&
            _config.ShowPrompt
        ) {
            _output->print(
                _config.Prompt.c_str()
            );
        }
    }

    void PrintHelp(
        std::string_view command = {}
    ) {
        if (_output == nullptr) {
            return;
        }

        command = Trim(command);

        if (!command.empty()) {
            for (
                const auto& registration :
                    _commands
            ) {
                if (
                    EqualsIgnoreCase(
                        registration.Name,
                        command
                    )
                ) {
                    _output->print(
                        registration.Name.c_str()
                    );

                    if (
                        !registration.Help.empty()
                    ) {
                        _output->print(" - ");
                        _output->println(
                            registration.Help.c_str()
                        );
                    } else {
                        _output->println();
                    }

                    return;
                }
            }

            _output->print(
                "Unknown command: "
            );

            _output->println(
                std::string(command).c_str()
            );

            return;
        }

        _output->println(
            "Available commands:"
        );

        _output->println(
            "  help [command]"
        );

        for (
            const auto& registration :
                _commands
        ) {
            _output->print("  ");
            _output->print(
                registration.Name.c_str()
            );

            if (
                !registration.Help.empty()
            ) {
                _output->print(" - ");
                _output->print(
                    registration.Help.c_str()
                );
            }

            _output->println();
        }
    }

public:
    Console() = default;

    bool Initialize(
        Stream& input,
        Print& output,
        const ConsoleConfig& config = {}
    ) {
        ConsoleConfig preparedConfig;
        std::string preparedLine;

        try {
            preparedConfig = config;
            preparedLine.reserve(
                preparedConfig.MaximumLineLength
            );
        } catch (...) {
            return false;
        }

        _input = &input;
        _output = &output;
        _config = std::move(preparedConfig);
        _line = std::move(preparedLine);
        _discardUntilNewline = false;

        PrintPrompt();
        return true;
    }

    void Shutdown() {
        _input = nullptr;
        _output = nullptr;
        _line.clear();
        _discardUntilNewline = false;
        _interceptors.clear();
    }

    bool GetIsInitialized() const noexcept {
        return
            _input != nullptr &&
            _output != nullptr;
    }

    Stream* GetInput() const noexcept {
        return _input;
    }

    Print* GetOutput() const noexcept {
        return _output;
    }

#ifdef ESPRESSIO_SERIAL_TESTING
    std::size_t __GetInputBufferCapacityForTesting() const noexcept {
        return _line.capacity();
    }
#endif

    uint32_t RegisterLineInterceptor(
        ConsoleLineInterceptor interceptor
    ) {
        if (!interceptor) {
            return 0;
        }

        const uint32_t id =
            _nextInterceptorID++;

        _interceptors.push_back(
            {
                id,
                std::move(interceptor)
            }
        );

        return id;
    }

    bool UnregisterLineInterceptor(
        uint32_t id
    ) {
        const auto found =
            std::remove_if(
                _interceptors.begin(),
                _interceptors.end(),
                [&](const auto& item) {
                    return item.ID == id;
                }
            );

        if (
            found == _interceptors.end()
        ) {
            return false;
        }

        _interceptors.erase(
            found,
            _interceptors.end()
        );

        return true;
    }

    bool RegisterCommand(
        std::string name,
        std::string help,
        ConsoleCommandHandler handler
    ) {
        if (
            name.empty() ||
            !handler
        ) {
            return false;
        }

        for (
            const auto& registration :
                _commands
        ) {
            if (
                EqualsIgnoreCase(
                    registration.Name,
                    name
                )
            ) {
                return false;
            }
        }

        _commands.push_back(
            {
                std::move(name),
                std::move(help),
                std::move(handler)
            }
        );

        std::sort(
            _commands.begin(),
            _commands.end(),
            [](
                const auto& left,
                const auto& right
            ) {
                return
                    left.Name <
                    right.Name;
            }
        );

        return true;
    }

    bool UnregisterCommand(
        std::string_view name
    ) {
        const auto found =
            std::remove_if(
                _commands.begin(),
                _commands.end(),
                [&](const auto& item) {
                    return
                        EqualsIgnoreCase(
                            item.Name,
                            name
                        );
                }
            );

        if (
            found == _commands.end()
        ) {
            return false;
        }

        _commands.erase(
            found,
            _commands.end()
        );

        return true;
    }

    ConsoleExecutionResult ExecuteLine(
        std::string_view line
    ) {
        if (_output == nullptr) {
            return
                ConsoleExecutionResult::
                    UnknownCommand;
        }

        line = Trim(line);

        if (line.empty()) {
            return
                ConsoleExecutionResult::Empty;
        }

        for (
            const auto& interceptor :
                _interceptors
        ) {
            if (
                interceptor.Handler &&
                interceptor.Handler(line)
            ) {
                return
                    ConsoleExecutionResult::
                        Executed;
            }
        }

        const auto separator =
            line.find_first_of(
                " \t"
            );

        const auto command =
            separator ==
                std::string_view::npos
                ? line
                : line.substr(
                    0,
                    separator
                );

        const auto arguments =
            separator ==
                std::string_view::npos
                ? std::string_view{}
                : Trim(
                    line.substr(
                        separator + 1
                    )
                );

        if (
            EqualsIgnoreCase(
                command,
                "help"
            )
        ) {
            PrintHelp(arguments);

            return
                ConsoleExecutionResult::
                    Executed;
        }

        for (
            const auto& registration :
                _commands
        ) {
            if (
                !EqualsIgnoreCase(
                    registration.Name,
                    command
                )
            ) {
                continue;
            }

            registration.Handler(
                {
                    command,
                    arguments
                }
            );

            return
                ConsoleExecutionResult::
                    Executed;
        }

        _output->print(
            "Unknown command: "
        );

        _output->println(
            std::string(command).c_str()
        );

        _output->println(
            "Type 'help' for available commands."
        );

        return
            ConsoleExecutionResult::
                UnknownCommand;
    }

    void Poll() {
        if (
            _input == nullptr ||
            _output == nullptr
        ) {
            return;
        }

        while (_input->available() > 0) {
            const int value =
                _input->read();

            if (value < 0) {
                break;
            }

            const char character =
                static_cast<char>(
                    value
                );

            if (
                character == '\r'
            ) {
                continue;
            }

            if (
                character == '\n'
            ) {
                if (_discardUntilNewline) {
                    _discardUntilNewline =
                        false;

                    _line.clear();

                    _output->println(
                        "Input rejected: line exceeds configured maximum length."
                    );

                    PrintPrompt();
                    continue;
                }

                if (_config.EchoInput) {
                    _output->println();
                }

                ExecuteLine(_line);
                _line.clear();

                PrintPrompt();
                continue;
            }

            if (
                character == '\b' ||
                character == 0x7F
            ) {
                if (!_line.empty()) {
                    _line.pop_back();

                    if (_config.EchoInput) {
                        _output->print(
                            "\b \b"
                        );
                    }
                }

                continue;
            }

            if (_discardUntilNewline) {
                continue;
            }

            if (
                _line.size() >=
                _config.MaximumLineLength
            ) {
                _discardUntilNewline =
                    true;

                continue;
            }

            _line.push_back(character);

            if (_config.EchoInput) {
                _output->write(
                    static_cast<uint8_t>(
                        character
                    )
                );
            }
        }
    }
};

}
