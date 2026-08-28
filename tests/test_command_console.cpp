#include <cassert>
#include <string>

#include <ESPressio_CommandConsole.hpp>
#include <ESPressio_CommandEvents.hpp>
#include <ESPressio_CommandResponseRoute.hpp>

class TestStream final : public ESPressio::System::IO::IByteStream {
public:
    std::string Output;

    std::size_t Available() const noexcept override { return 0; }

    ESPressio::System::PlatformResult Read(uint8_t&) noexcept override {
        return ESPressio::System::PlatformResult::Failed(
            ESPressio::System::PlatformStatus::Unavailable
        );
    }

    ESPressio::System::PlatformResult Write(
        const uint8_t* data,
        std::size_t size,
        std::size_t& bytesWritten
    ) noexcept override {
        bytesWritten = 0;
        if (data == nullptr && size != 0) {
            return ESPressio::System::PlatformResult::Failed(
                ESPressio::System::PlatformStatus::InvalidArgument
            );
        }
        try {
            if (size != 0) {
                Output.append(reinterpret_cast<const char*>(data), size);
            }
            bytesWritten = size;
            return ESPressio::System::PlatformResult::Succeeded();
        } catch (...) {
            return ESPressio::System::PlatformResult::Failed(
                ESPressio::System::PlatformStatus::OutOfMemory
            );
        }
    }
};

int main() {
    using namespace ESPressio;

    TestStream stream;
    Serial::Console console;
    Serial::ConsoleConfig config;
    config.ShowPrompt = false;
    assert(console.Initialize(stream, stream, config));

    bool legacyCalled = false;
    assert(console.RegisterCommand(
        "legacy",
        "Legacy command",
        [&](const Serial::ConsoleCommandContext&) { legacyCalled = true; }
    ));

    int pingExecutions = 0;
    Command::CommandRegistry registry;
    auto& ping = registry.Command("ping");
    ping.OnExecute([&](const Command::CommandContext&) {
        ++pingExecutions;
        return Command::CommandResult::Ok("pong");
    });

    Serial::CommandConsole commandConsole;
    assert(commandConsole.Initialize(console, registry));

    bool envelopeQueued = false;
    Command::CommandRequestEnvelope captured;
    Event::InboundCommandEvent::OnQueue =
        [&](const Command::CommandRequestEnvelope& envelope) {
            envelopeQueued = true;
            captured = envelope;
        };

    // Transport-style serial ingress is accepted synchronously but application
    // Command execution is deferred through an InboundCommandEvent envelope.
    assert(console.ExecuteLine("ping") == Serial::ConsoleExecutionResult::Executed);
    assert(envelopeQueued);
    assert(captured.RequestId != 0);
    assert(captured.Origin.TransportRoute != 0);
    assert(captured.RawString() == "ping");
    assert(pingExecutions == 0);
    assert(stream.Output.find("pong") == std::string::npos);

    // Completion can happen after the serial interceptor stack has unwound and
    // is routed back through the stored lifetime-safe response route.
    Command::CommandResponseEnvelope response;
    response.RequestId = captured.RequestId;
    response.Success = true;
    response.Code = 0;
    assert(response.SetMessage("pong"));
    assert(Command::CommandResponseRouteRegistry::GetInstance().Route(
        captured.Origin,
        response
    ));
    assert(stream.Output.find("pong") != std::string::npos);
    assert(pingExecutions == 0);

    // Explicit programmatic Execute remains the local/direct synchronous path.
    stream.Output.clear();
    const auto direct = commandConsole.Execute("ping");
    assert(direct.success);
    assert(direct.message == "pong");
    assert(pingExecutions == 1);

    // Interceptor chaining remains intact for legacy Console commands.
    stream.Output.clear();
    assert(console.ExecuteLine("legacy") == Serial::ConsoleExecutionResult::Executed);
    assert(legacyCalled);

    auto registration = registry.RegisterCommand("temporary");
    assert(registration.Active());
    registry.Command("temporary").OnExecute([](const Command::CommandContext&) {
        return Command::CommandResult::Ok("temporary result");
    });
    registration.Reset();
    stream.Output.clear();
    console.ExecuteLine("temporary");
    assert(stream.Output.find("Unknown command") != std::string::npos);

    // After shutdown the response route is unavailable rather than retaining a
    // dangling CommandConsole pointer.
    const auto oldOrigin = captured.Origin;
    commandConsole.Shutdown();
    assert(!commandConsole.GetIsInitialized());
    assert(!Command::CommandResponseRouteRegistry::GetInstance().Route(
        oldOrigin,
        response
    ));

    Event::InboundCommandEvent::OnQueue = {};
    return 0;
}
