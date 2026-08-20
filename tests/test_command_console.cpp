#include <cassert>
#include <string>

#include <ESPressio_CommandConsole.hpp>

class TestStream final : public Stream {
public:
    std::string Output;
    std::size_t write(uint8_t value) override {
        Output.push_back(static_cast<char>(value));
        return 1;
    }
    int available() override { return 0; }
    int read() override { return -1; }
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

    Command::CommandRegistry registry;
    auto& ping = registry.Command("ping");
    ping.OnExecute([](const Command::CommandContext&) {
        return Command::CommandResult::Ok("pong");
    });

    Serial::CommandConsole commandConsole;
    assert(commandConsole.Initialize(console, registry));

    assert(console.ExecuteLine("ping") == Serial::ConsoleExecutionResult::Executed);
    assert(stream.Output.find("pong") != std::string::npos);

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

    commandConsole.Shutdown();
    assert(!commandConsole.GetIsInitialized());
    return 0;
}
