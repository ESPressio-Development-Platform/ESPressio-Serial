#pragma once

#if !__has_include(<ESPressio_Command.hpp>)
#error "ESPressio CommandConsole requires ESPressio Command."
#endif

#include <atomic>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <Arduino.h>
#include <ESPressio_Command.hpp>
#include <ESPressio_CommandEnvelope.hpp>
#include <ESPressio_CommandEvents.hpp>
#include <ESPressio_CommandResponseRoute.hpp>

#include "../console/ESPressio_Console.hpp"

namespace ESPressio::Serial {

/// <summary>Integrates ESPressio Command parsing/execution with the line-oriented Serial Console.</summary>
/// <remarks>Recognized console lines enter Command through the asynchronous inbound Event/envelope path; explicit Execute calls remain synchronous local invocations.</remarks>
class CommandConsole final {
private:
    class ResponseRoute final : public Command::ICommandResponseRoute {
        CommandConsole* _owner = nullptr;

    public:
        explicit ResponseRoute(CommandConsole& owner)
            : _owner(&owner) {
        }

        void Detach() {
            _owner = nullptr;
        }

        bool SendCommandResponse(
            const Command::CommandOriginAddress&,
            const Command::CommandResponseEnvelope& response
        ) override {
            if (_owner == nullptr) {
                return false;
            }
            _owner->PrintResponse(response);
            return true;
        }
    };

    Console* _console = nullptr;
    Command::CommandRegistry* _registry = nullptr;
    Print* _output = nullptr;
    uint32_t _interceptorID = 0;
    std::shared_ptr<ResponseRoute> _responseRoute;
    Command::CommandTransportRouteId _responseRouteId = 0;
    std::atomic<Command::CommandRequestId> _nextRequestId{1};

    bool CanHandleLine(std::string_view line) const {
        if (_registry == nullptr) {
            return false;
        }

        std::string error;
        const auto tokens = Command::TextCommandParser::Tokenize(
            std::string(line),
            &error
        );
        if (!error.empty() || tokens.empty()) {
            return false;
        }
        if (tokens.front() == "help" || tokens.front() == "?") {
            return true;
        }

        return _registry->Resolve({tokens.front()}) != nullptr;
    }

    void PrintResult(const Command::CommandResult& result) {
        if (_output == nullptr || result.message.empty()) {
            return;
        }
        _output->println(result.message.c_str());
    }

    void PrintResponse(const Command::CommandResponseEnvelope& response) {
        if (_output == nullptr || response.MessageLength == 0) {
            return;
        }
        const std::string message = response.MessageString();
        _output->println(message.c_str());
    }

    bool HandleLine(std::string_view line) {
        if (!CanHandleLine(line) || _responseRouteId == 0) {
            return false;
        }

        Command::CommandRequestEnvelope envelope;
        envelope.RequestId =
            _nextRequestId.fetch_add(1, std::memory_order_relaxed);
        if (envelope.RequestId == 0) {
            envelope.RequestId =
                _nextRequestId.fetch_add(1, std::memory_order_relaxed);
        }
        envelope.Origin.TransportRoute = _responseRouteId;
        envelope.ResponseExpectation =
            Command::CommandResponseExpectation::Completion;
        envelope.ResponseMode = Command::CommandResponseMode::Single;
        envelope.ResponseTimeoutMilliseconds = 100;

        if (!envelope.SetRaw(line.data(), line.size())) {
            if (_output != nullptr) {
                _output->println("Command input exceeds asynchronous envelope capacity");
            }
            return true;
        }

        (new Event::InboundCommandEvent(envelope))->Queue();
        return true;
    }

public:
    CommandConsole() = default;
    CommandConsole(const CommandConsole&) = delete;
    CommandConsole& operator=(const CommandConsole&) = delete;
    ~CommandConsole() { Shutdown(); }

    /// <summary>Attaches Command handling to an initialized Console and registers a response route for asynchronous completions.</summary>
    /// <param name="console">Initialized console whose lines will be intercepted.</param>
    /// <param name="registry">Command registry used for recognition and direct execution.</param>
    /// <returns>True when both the response route and line interceptor are registered.</returns>
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

        _responseRoute = std::make_shared<ResponseRoute>(*this);
        _responseRouteId =
            Command::CommandResponseRouteRegistry::GetInstance().Register(
                _responseRoute
            );
        if (_responseRouteId == 0) {
            Shutdown();
            return false;
        }

        _interceptorID = console.RegisterLineInterceptor(
            [this](std::string_view line) { return HandleLine(line); }
        );
        if (_interceptorID == 0) {
            Shutdown();
            return false;
        }
        return true;
    }

    /// <summary>Removes the Console interceptor and Command response route and clears all non-owning references.</summary>
    void Shutdown() {
        if (_console != nullptr && _interceptorID != 0) {
            _console->UnregisterLineInterceptor(_interceptorID);
        }
        _interceptorID = 0;

        if (_responseRouteId != 0) {
            Command::CommandResponseRouteRegistry::GetInstance().Unregister(
                _responseRouteId
            );
            _responseRouteId = 0;
        }
        if (_responseRoute) {
            _responseRoute->Detach();
            _responseRoute.reset();
        }

        _console = nullptr;
        _registry = nullptr;
        _output = nullptr;
    }

    /// <summary>Reports whether the Console, Command registry, interceptor, and response route are all attached.</summary>
    bool GetIsInitialized() const noexcept {
        return
            _console != nullptr &&
            _registry != nullptr &&
            _interceptorID != 0 &&
            _responseRouteId != 0;
    }

    /// <summary>Returns the attached non-owning Command registry.</summary>
    Command::CommandRegistry* GetRegistry() const noexcept { return _registry; }
    /// <summary>Returns the attached non-owning Console.</summary>
    Console* GetConsole() const noexcept { return _console; }

    /// <summary>Executes a Command synchronously through the direct/local registry path and prints any result message.</summary>
    /// <remarks>This method intentionally bypasses the asynchronous transport-style console ingress path.</remarks>
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
