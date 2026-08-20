#pragma once

#if !__has_include(<ESPressio_Command.hpp>)
#error "ESPressio CommandConsole requires ESPressio Command >= 0.2.0 < 1.0.0."
#endif

#include <string>
#include <string_view>

#include <Arduino.h>
#include <ESPressio_Command.hpp>

#include "../console/ESPressio_Console.hpp"

namespace ESPressio::Serial {

class CommandConsole final {
private:
    Console* _console = nullptr;
    Command::CommandRegistry* _registry = nullptr;
    Print* _output = nullptr;
    uint32_t _interceptorID = 0;

    static bool IsUnknownCommand(const Command::CommandResult& result) {
        return !result.success &&
            result.message.rfind("Unknown command '", 0) == 0;
    }

    void PrintResult(const Command::CommandResult& result) {
        if (_output == nullptr || result.message.empty()) {
            return;
        }
        _output->println(result.message.c_str());
    }

    bool HandleLine(std::string_view line) {
        if (_registry == nullptr) {
            return false;
        }
        auto result = _registry->Invoke(std::string(line));
        if (IsUnknownCommand(result)) {
            return false;
        }
        PrintResult(result);
        return true;
    }

public:
    CommandConsole() = default;
    CommandConsole(const CommandConsole&) = delete;
    CommandConsole& operator=(const CommandConsole&) = delete;
    ~CommandConsole() { Shutdown(); }

    bool Initialize(
        Console& console,
        Command::CommandRegistry& registry = Command::CommandRegistry::GetInstance()
    ) {
        Shutdown();
        if (!console.GetIsInitialized() || console.GetOutput() == nullptr) {
            return false;
        }
        _console = &console;
        _registry = &registry;
        _output = console.GetOutput();
        _interceptorID = console.RegisterLineInterceptor(
            [this](std::string_view line) { return HandleLine(line); }
        );
        if (_interceptorID == 0) {
            _console = nullptr;
            _registry = nullptr;
            _output = nullptr;
            return false;
        }
        return true;
    }

    void Shutdown() {
        if (_console != nullptr && _interceptorID != 0) {
            _console->UnregisterLineInterceptor(_interceptorID);
        }
        _interceptorID = 0;
        _console = nullptr;
        _registry = nullptr;
        _output = nullptr;
    }

    bool GetIsInitialized() const noexcept {
        return _console != nullptr && _registry != nullptr && _interceptorID != 0;
    }

    Command::CommandRegistry* GetRegistry() const noexcept { return _registry; }
    Console* GetConsole() const noexcept { return _console; }

    Command::CommandResult Execute(std::string_view line) {
        if (_registry == nullptr) {
            return Command::CommandResult::Error("CommandConsole is not initialized");
        }
        auto result = _registry->Invoke(std::string(line));
        PrintResult(result);
        return result;
    }
};

} // namespace ESPressio::Serial
