#include <cassert>
#include <string>

#include <ESPressio_EventConsole.hpp>

class TestStream final :
    public ESPressio::System::IO::IByteStream {

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

    Serial::ConsoleConfig
        consoleConfig;

    consoleConfig.ShowPrompt =
        false;

    assert(
        console.Initialize(
            stream,
            stream,
            consoleConfig
        )
    );

    Serial::EventConsole
        eventConsole;

    Serial::EventConsoleConfig
        config;

    config.RequireConfirmation =
        true;

    assert(
        eventConsole.Initialize(
            console,
            config,
            Event::
                EventTransportManager::
                    GetInstance()
        )
    );

    assert(
        eventConsole.AllowEvent(
            "test.event.v1"
        )
    );

    console.ExecuteLine(
        "events"
    );

    assert(
        stream.Output.find(
            "test.event.v1"
        ) !=
        std::string::npos
    );

    stream.Output.clear();

    console.ExecuteLine(
        "event describe test.event.v1"
    );

    assert(
        stream.Output.find(
            "value : integer required"
        ) !=
        std::string::npos
    );

    Event::
        EventTransportManager::
            DispatchCount = 0;

    console.ExecuteLine(
        "event queue test.event.v1 {\"value\":42}"
    );

    assert(
        Event::
            EventTransportManager::
                DispatchCount == 0
    );

    assert(
        stream.Output.find(
            "Dispatch Event"
        ) !=
        std::string::npos
    );

    console.ExecuteLine("y");

    assert(
        Event::
            EventTransportManager::
                DispatchCount == 1
    );

    eventConsole.SetAccessPolicy(
        Serial::
            EventConsoleAccessPolicy::
                AllowListedOnly
    );

    eventConsole.ClearAccessLists();

    stream.Output.clear();

    console.ExecuteLine(
        "event queue test.event.v1 {\"value\":43}"
    );

    assert(
        Event::
            EventTransportManager::
                DispatchCount == 1
    );

    assert(
        stream.Output.find(
            "not permitted"
        ) !=
        std::string::npos
    );

    return 0;
}
