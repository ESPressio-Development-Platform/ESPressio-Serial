#include <cassert>
#include <cstring>
#include <limits>
#include <string>

#include <ESPressio_Console.hpp>

class TestStream final :
    public ESPressio::System::IO::IByteStream {

public:
    std::string Input;
    std::string Output;
    std::size_t ReadOffset = 0;

    std::size_t Available() const noexcept override {
        return Input.size() - ReadOffset;
    }

    ESPressio::System::PlatformResult Read(uint8_t& value) noexcept override {
        if (ReadOffset >= Input.size()) {
            return ESPressio::System::PlatformResult::Failed(
                ESPressio::System::PlatformStatus::Unavailable
            );
        }
        value = static_cast<uint8_t>(Input[ReadOffset++]);
        return ESPressio::System::PlatformResult::Succeeded();
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
    using namespace ESPressio::Serial;

    TestStream stream;

    {
        Console impossibleConsole;
        ConsoleConfig impossibleConfig;
        impossibleConfig.ShowPrompt = false;
        impossibleConfig.MaximumLineLength =
            std::numeric_limits<std::size_t>::max();

        assert(
            !impossibleConsole.Initialize(
                stream,
                stream,
                impossibleConfig
            )
        );

        assert(
            !impossibleConsole.GetIsInitialized()
        );
    }

    Console console;

    ConsoleConfig config;
    config.ShowPrompt = false;
    config.MaximumLineLength = 128;

    assert(
        console.Initialize(
            stream,
            stream,
            config
        )
    );

    const auto reservedCapacity =
        console.__GetInputBufferCapacityForTesting();

    assert(
        reservedCapacity >=
        config.MaximumLineLength
    );

    bool called = false;
    std::string lastArguments;

    assert(
        console.RegisterCommand(
            "echo",
            "echo test",
            [&](const auto& context) {
                called = true;
                lastArguments =
                    std::string(
                        context.Arguments
                    );
            }
        )
    );

    assert(
        console.ExecuteLine(
            "echo hello world"
        ) ==
        ConsoleExecutionResult::Executed
    );

    assert(called);
    assert(lastArguments == "hello world");

    bool firstInterceptor = false;
    bool secondInterceptor = false;

    const auto first =
        console.RegisterLineInterceptor(
            [&](std::string_view line) {
                firstInterceptor =
                    line == "intercept";

                return false;
            }
        );

    const auto second =
        console.RegisterLineInterceptor(
            [&](std::string_view line) {
                secondInterceptor =
                    line == "intercept";

                return
                    secondInterceptor;
            }
        );

    assert(first != 0);
    assert(second != 0);

    assert(
        console.ExecuteLine(
            "intercept"
        ) ==
        ConsoleExecutionResult::Executed
    );

    assert(firstInterceptor);
    assert(secondInterceptor);

    assert(
        console.UnregisterLineInterceptor(
            second
        )
    );

    stream.Input =
        "echo from poll\n";

    stream.ReadOffset = 0;
    called = false;

    console.Poll();

    assert(called);
    assert(lastArguments == "from poll");
    assert(
        console.__GetInputBufferCapacityForTesting() ==
        reservedCapacity
    );

    stream.Input =
        std::string(
            config.MaximumLineLength,
            'x'
        ) +
        "\n";

    stream.ReadOffset = 0;
    console.Poll();

    assert(
        console.__GetInputBufferCapacityForTesting() ==
        reservedCapacity
    );

    stream.Input =
        std::string(
            config.MaximumLineLength + 1,
            'y'
        ) +
        "\n";

    stream.ReadOffset = 0;
    stream.Output.clear();
    console.Poll();

    assert(
        stream.Output.find(
            "Input rejected: line exceeds configured maximum length."
        ) !=
        std::string::npos
    );

    assert(
        console.__GetInputBufferCapacityForTesting() ==
        reservedCapacity
    );

    return 0;
}
