#include <cassert>
#include <string>

#include <ESPressio_SerialLogging.hpp>

namespace {

class TestOutput final : public ESPressio::System::IO::IByteOutput {
public:
    std::string Output;

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
            Output.append(reinterpret_cast<const char*>(data), size);
            bytesWritten = size;
            return ESPressio::System::PlatformResult::Succeeded();
        } catch (...) {
            return ESPressio::System::PlatformResult::Failed(
                ESPressio::System::PlatformStatus::OutOfMemory
            );
        }
    }
};

} // namespace

int main() {
    using namespace ESPressio;

    TestOutput output;
    Serial::SerialLogSink sink(output);

    constexpr auto category = Logging::LogCategory::Named("Laser-Trigger");
    assert(sink.IsEnabled(Logging::LogLevel::Info, category));

    sink.SetLevelMask(
        Logging::LogLevelBit(Logging::LogLevel::Error) |
        Logging::LogLevelBit(Logging::LogLevel::Fatal)
    );
    assert(!sink.IsEnabled(Logging::LogLevel::Info, category));
    assert(sink.IsEnabled(Logging::LogLevel::Error, category));

    const Logging::LogField metadata[] = {
        {"channel", uint32_t{6}},
        {"armed", true},
        {"label", std::string_view{"north"}}
    };
    const char message[] = "triggered";
    const Logging::LogRecordView view{
        Logging::LogTimestamp{
            123456789ULL,
            1700000000000000000ULL,
            Timing::ClockSynchronizationState::Synchronized
        },
        Logging::LogLevel::Error,
        category,
        std::string_view{message},
        Logging::Fields(metadata)
    };

    const Logging::LogRecordLease lease(view);
    sink.Accept(lease);

    assert(output.Output ==
        "[mono=123456789ns system=1700000000000000000ns] [ERROR] [Laser-Trigger] "
        "triggered channel=6 armed=true label=north\r\n"
    );

    return 0;
}
