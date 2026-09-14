#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>

#include <ESPressio_CommandConsole.hpp>
#include <ESPressio_CommandDescriptor.hpp>
#include <ESPressio_TypeDirectory.hpp>

using namespace ESPressio;

namespace {

class TestStream final : public System::IO::IByteStream {
public:
    std::string Output;
    std::size_t Available() const noexcept override { return 0; }
    System::PlatformResult Read(std::uint8_t&) noexcept override {
        return System::PlatformResult::Failed(System::PlatformStatus::Unavailable);
    }
    System::PlatformResult Write(const std::uint8_t* data, std::size_t size, std::size_t& bytesWritten) noexcept override {
        bytesWritten = 0;
        if (data == nullptr && size != 0) return System::PlatformResult::Failed(System::PlatformStatus::InvalidArgument);
        try {
            if (size != 0) Output.append(reinterpret_cast<const char*>(data), size);
            bytesWritten = size;
            return System::PlatformResult::Succeeded();
        } catch (...) {
            return System::PlatformResult::Failed(System::PlatformStatus::OutOfMemory);
        }
    }
    void Clear() { Output.clear(); }
};

std::uint32_t Submissions = 0;

Command::CommandSubmissionResult SubmitCommand(Command::CommandPayloadFormat format, const std::uint8_t* payload, std::size_t size) noexcept {
    assert(format == Command::CommandPayloadFormat::JSON);
    assert(std::string_view(reinterpret_cast<const char*>(payload), size) == R"({"value":7})");
    ++Submissions;
    return {Command::CommandSubmissionStatus::Accepted, Command::CommandId{1}};
}

const Serializable::StaticSchemaDescriptor& TestSchema() {
    static const Serializable::StaticSchemaDescriptor schema = [] {
        Serializable::StaticSchemaDescriptor value{};
        value.CurrentVersion = 1;
        value.MinimumReadableVersion = 1;
        value.MaximumReadableVersion = 1;
        value.MaximumDirectBinaryBytes = 16;
        value.MaximumCborBytes = 24;
        value.MaximumJsonBytes = 32;
        return value;
    }();
    return schema;
}

Primitive::PrimitiveTypeDescriptor FireAndForgetDescriptor() {
    static Command::CommandTypeDescriptor extension{};
    extension.TypeId = Command::CommandTypeId{0xA101};
    extension.Tier = Command::CommandTier::Serializable;
    extension.MaximumLiveInstances = 2;
    extension.MaximumPendingExecutions = 1;
    extension.ExecutionLaneCount = 1;
    extension.RequestSchema = &TestSchema();
    extension.DynamicConstruction = Command::CommandDynamicConstructionMode::FireAndForget;
    extension.SubmitSerialized = &SubmitCommand;
    return {{Command::CommandFamilyId, extension.TypeId.Value()}, "Test.Serial.Command", Primitive::PrimitiveTypeCapabilities{1}, {1,1}, {}, {64}, {&extension}};
}

Primitive::PrimitiveTypeDescriptor RequesterRequiredDescriptor() {
    static Command::CommandTypeDescriptor extension{};
    extension.TypeId = Command::CommandTypeId{0xA102};
    extension.Tier = Command::CommandTier::Serializable;
    extension.RequestSchema = &TestSchema();
    extension.DynamicConstruction = Command::CommandDynamicConstructionMode::RequesterRequired;
    return {{Command::CommandFamilyId, extension.TypeId.Value()}, "Test.Serial.RequestCommand", Primitive::PrimitiveTypeCapabilities{1}, {1,1}, {}, {64}, {&extension}};
}

class Authorizer final : public Serial::ICommandConsoleAuthorizer {
public:
    bool Allowed = true;
    mutable std::uint32_t Calls = 0;
    Serial::CommandConsoleAuthorizationDecision Authorize(const Primitive::PrimitiveTypeDescriptor&) const noexcept override {
        ++Calls;
        return Allowed ? Serial::CommandConsoleAuthorizationDecision::Authorized : Serial::CommandConsoleAuthorizationDecision::Denied;
    }
};

} // namespace

int main() {
    Primitive::TypeDirectory<2> directory;
    assert(directory.Register(FireAndForgetDescriptor()) == Primitive::TypeDirectoryRegistrationStatus::Success);
    assert(directory.Register(RequesterRequiredDescriptor()) == Primitive::TypeDirectoryRegistrationStatus::Success);
    assert(directory.Initialize() == Primitive::TypeDirectoryInitializationStatus::Success);

    TestStream stream;
    Serial::Console console;
    Serial::ConsoleConfig config;
    config.ShowPrompt = false;
    config.MaximumLineLength = 256;
    assert(console.Initialize(stream, stream, config));

    bool legacyCalled = false;
    assert(console.RegisterCommand("legacy", "unrelated Console command", [&](const Serial::ConsoleCommandContext&) { legacyCalled = true; }));

    Authorizer authorizer;
    Serial::CommandConsole commandConsole;
    assert(commandConsole.Initialize(console, directory.View(), authorizer));
    assert(commandConsole.GetIsInitialized());

    assert(console.ExecuteLine("commands") == Serial::ConsoleExecutionResult::Executed);
    assert(stream.Output.find("Test.Serial.Command") != std::string::npos);
    assert(stream.Output.find("Test.Serial.RequestCommand") != std::string::npos);

    stream.Clear();
    assert(console.ExecuteLine("command describe Test.Serial.Command") == Serial::ConsoleExecutionResult::Executed);
    assert(stream.Output.find("FireAndForget") != std::string::npos);
    assert(stream.Output.find("JSON request bytes: 32") != std::string::npos);

    stream.Clear();
    assert(console.ExecuteLine(R"(command json Test.Serial.Command {"value":7})") == Serial::ConsoleExecutionResult::Executed);
    assert(Submissions == 1);
    assert(stream.Output.find("Command submission: Accepted") != std::string::npos);

    stream.Clear();
    assert(console.ExecuteLine(R"(command json Test.Serial.RequestCommand {"value":7})") == Serial::ConsoleExecutionResult::Executed);
    assert(Submissions == 1);
    assert(stream.Output.find("requester capability") != std::string::npos);

    authorizer.Allowed = false;
    stream.Clear();
    assert(console.ExecuteLine(R"(command json Test.Serial.Command {"value":7})") == Serial::ConsoleExecutionResult::Executed);
    assert(Submissions == 1);
    assert(stream.Output.find("not authorized") != std::string::npos);
    authorizer.Allowed = true;

    stream.Clear();
    assert(console.ExecuteLine("command json Test.Serial.Command 123456789012345678901234567890123") == Serial::ConsoleExecutionResult::Executed);
    assert(Submissions == 1);
    assert(stream.Output.find("exceeds bounded schema capacity") != std::string::npos);

    stream.Clear();
    assert(console.ExecuteLine("legacy") == Serial::ConsoleExecutionResult::Executed);
    assert(legacyCalled);

    commandConsole.Shutdown();
    assert(!commandConsole.GetIsInitialized());
    stream.Clear();
    assert(console.ExecuteLine("command describe Test.Serial.Command") == Serial::ConsoleExecutionResult::UnknownCommand);
    return 0;
}
