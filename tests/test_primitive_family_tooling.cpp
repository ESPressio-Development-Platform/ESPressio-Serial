#include <cassert>
#include <cstring>
#include <string>
#include <string_view>

#include <ESPressio_CommandConsole.hpp>
#include <ESPressio_CommandMonitor.hpp>
#include <ESPressio_EventConsole.hpp>
#include <ESPressio_EventMonitor.hpp>
#include <ESPressio_StateMonitor.hpp>

using namespace ESPressio;

namespace {

class TestStream final : public System::IO::IByteStream {
public:
    std::string Output;
    std::size_t Available() const noexcept override { return 0; }
    System::PlatformResult Read(std::uint8_t&) noexcept override {
        return System::PlatformResult::Failed(System::PlatformStatus::Unavailable);
    }
    System::PlatformResult Write(const std::uint8_t* data, std::size_t size, std::size_t& written) noexcept override {
        written = 0;
        if (!data && size) return System::PlatformResult::Failed(System::PlatformStatus::InvalidArgument);
        try {
            Output.append(reinterpret_cast<const char*>(data), size);
            written = size;
            return System::PlatformResult::Succeeded();
        } catch (...) {
            return System::PlatformResult::Failed(System::PlatformStatus::OutOfMemory);
        }
    }
    void Clear() { Output.clear(); }
};

std::uint32_t CommandSubmissions = 0;
std::uint32_t EventDispatches = 0;

Command::CommandSubmissionResult SubmitCommand(
    Command::CommandPayloadFormat format,
    const std::uint8_t* payload,
    std::size_t size
) noexcept {
    assert(format == Command::CommandPayloadFormat::JSON);
    assert(std::string_view(reinterpret_cast<const char*>(payload), size) == R"({"value":7})");
    ++CommandSubmissions;
    return {Command::CommandSubmissionStatus::Accepted, Command::CommandId{1}};
}

Event::EventDynamicDispatchResult DispatchEvent(
    Event::EventPayloadFormat format,
    const std::uint8_t* payload,
    std::size_t size
) noexcept {
    assert(format == Event::EventPayloadFormat::JSON);
    assert(std::string_view(reinterpret_cast<const char*>(payload), size) == R"({"value":8})");
    ++EventDispatches;
    return {Event::EventDynamicDispatchStatus::Accepted, Primitive::ConceptualMessageId{1}};
}

State::StateDynamicReadResult ReadStateJson(std::uint8_t* output, std::size_t capacity) {
    constexpr std::string_view Json = R"({"value":42})";
    if (capacity < Json.size()) {
        return {State::StateDynamicReadStatus::InsufficientOutput, 0, {0x0000000200000003ULL, Timing::TimeReliability::Holdover}};
    }
    std::memcpy(output, Json.data(), Json.size());
    return {State::StateDynamicReadStatus::Success, Json.size(), {0x0000000200000003ULL, Timing::TimeReliability::Holdover}};
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

Primitive::PrimitiveTypeDescriptor CommandDescriptor() {
    static Command::CommandTypeDescriptor extension{};
    extension.TypeId = Command::CommandTypeId{0x9101};
    extension.Tier = Command::CommandTier::Serializable;
    extension.MaximumLiveInstances = 2;
    extension.MaximumPendingExecutions = 1;
    extension.ExecutionLaneCount = 1;
    extension.RequestSchema = &TestSchema();
    extension.DynamicConstruction = Command::CommandDynamicConstructionMode::FireAndForget;
    extension.SubmitSerialized = &SubmitCommand;
    return {{Command::CommandFamilyId, extension.TypeId.Value()}, "Test.Serial.Command",
        Primitive::PrimitiveTypeCapabilities{1}, {1,1}, {}, {64}, {&extension}};
}

Primitive::PrimitiveTypeDescriptor RequesterCommandDescriptor() {
    static Command::CommandTypeDescriptor extension{};
    extension.TypeId = Command::CommandTypeId{0x9102};
    extension.Tier = Command::CommandTier::Serializable;
    extension.RequestSchema = &TestSchema();
    extension.DynamicConstruction = Command::CommandDynamicConstructionMode::RequesterRequired;
    return {{Command::CommandFamilyId, extension.TypeId.Value()}, "Test.Serial.RequestCommand",
        Primitive::PrimitiveTypeCapabilities{1}, {1,1}, {}, {64}, {&extension}};
}

Primitive::PrimitiveTypeDescriptor EventDescriptor() {
    static Event::EventTypeDescriptor extension{};
    extension.TypeId = Event::EventTypeId{0x9201};
    extension.Tier = Event::EventTier::Serializable;
    extension.MaximumLiveInstances = 2;
    extension.MaximumPendingInstances = 1;
    extension.Schema = &TestSchema();
    extension.DynamicallyConstructible = true;
    extension.DispatchSerialized = &DispatchEvent;
    return {{Event::EventFamilyId, extension.TypeId.Value()}, "Test.Serial.Event",
        Primitive::PrimitiveTypeCapabilities{1}, {1,1}, {}, {64}, {&extension}};
}

Primitive::PrimitiveTypeDescriptor StateDescriptor() {
    static State::StateTypeDescriptor extension{};
    extension.TypeId = State::StateTypeId{0x9301};
    extension.Tier = State::StateTier::Serializable;
    extension.ValueBytes = 4;
    extension.RuntimeBytes = 16;
    extension.ValueSchema = &TestSchema();
    extension.MaximumSerializedValueBytes = {16,24,32};
    extension.ReadValue[2] = &ReadStateJson;
    return {{State::StateFamilyId, extension.TypeId.Value()}, "Test.Serial.State",
        Primitive::PrimitiveTypeCapabilities{1}, {1,1}, {}, {64}, {&extension}};
}

Primitive::PrimitiveTypeDescriptor LocalStateDescriptor() {
    static State::StateTypeDescriptor extension{};
    extension.TypeId = State::StateTypeId{0x9302};
    extension.Tier = State::StateTier::Local;
    return {{State::StateFamilyId, extension.TypeId.Value()}, "Test.Serial.LocalState",
        Primitive::PrimitiveTypeCapabilities{}, {1,1}, {}, {0}, {&extension}};
}

class CommandAuth final : public Serial::ICommandConsoleAuthorizer {
public:
    bool Allowed = true;
    mutable std::uint32_t Calls = 0;
    Serial::CommandConsoleAuthorizationDecision Authorize(const Primitive::PrimitiveTypeDescriptor&) const noexcept override {
        ++Calls;
        return Allowed ? Serial::CommandConsoleAuthorizationDecision::Authorized : Serial::CommandConsoleAuthorizationDecision::Denied;
    }
};

class EventAuth final : public Serial::IEventConsoleAuthorizer {
public:
    bool Allowed = true;
    mutable std::uint32_t Calls = 0;
    Serial::EventConsoleAuthorizationDecision Authorize(const Primitive::PrimitiveTypeDescriptor&) const noexcept override {
        ++Calls;
        return Allowed ? Serial::EventConsoleAuthorizationDecision::Authorized : Serial::EventConsoleAuthorizationDecision::Denied;
    }
};

class StateAuth final : public Serial::IStateMonitorAuthorizer {
public:
    bool Allowed = true;
    mutable std::uint32_t Calls = 0;
    Serial::StateMonitorAuthorizationDecision Authorize(const Primitive::PrimitiveTypeDescriptor&) const noexcept override {
        ++Calls;
        return Allowed ? Serial::StateMonitorAuthorizationDecision::Authorized : Serial::StateMonitorAuthorizationDecision::Denied;
    }
};

} // namespace

int main() {
    Primitive::TypeDirectory<5> directory;
    assert(directory.Register(CommandDescriptor()) == Primitive::TypeDirectoryRegistrationStatus::Success);
    assert(directory.Register(RequesterCommandDescriptor()) == Primitive::TypeDirectoryRegistrationStatus::Success);
    assert(directory.Register(EventDescriptor()) == Primitive::TypeDirectoryRegistrationStatus::Success);
    assert(directory.Register(StateDescriptor()) == Primitive::TypeDirectoryRegistrationStatus::Success);
    assert(directory.Register(LocalStateDescriptor()) == Primitive::TypeDirectoryRegistrationStatus::Success);
    assert(directory.Initialize() == Primitive::TypeDirectoryInitializationStatus::Success);

    TestStream stream;
    Serial::Console console;
    Serial::ConsoleConfig config;
    config.ShowPrompt = false;
    config.MaximumLineLength = 256;
    assert(console.Initialize(stream, stream, config));

    CommandAuth commandAuth;
    EventAuth eventAuth;
    StateAuth stateAuth;

    Serial::CommandMonitor commandMonitor;
    Serial::EventMonitor eventMonitor;
    Serial::StateMonitor<64> stateMonitor;
    assert(commandMonitor.Initialize(directory.View(), *console.GetOutput()));
    assert(eventMonitor.Initialize(directory.View(), *console.GetOutput()));
    assert(stateMonitor.Initialize(directory.View(), *console.GetOutput(), stateAuth));

    commandMonitor.List();
    eventMonitor.List();
    stateMonitor.List();
    assert(stream.Output.find("Test.Serial.Command") != std::string::npos);
    assert(stream.Output.find("Test.Serial.Event") != std::string::npos);
    assert(stream.Output.find("Test.Serial.State") != std::string::npos);
    stream.Clear();

    assert(commandMonitor.Describe("Test.Serial.Command"));
    assert(eventMonitor.Describe("Test.Serial.Event"));
    assert(stateMonitor.Describe("Test.Serial.State"));

    assert(stateMonitor.ReadJson("Test.Serial.State") == State::StateDynamicReadStatus::Success);
    assert(stream.Output.find(R"({"value":42})") != std::string::npos);
    assert(stream.Output.find("8589934595") != std::string::npos);
    assert(stream.Output.find("Holdover") != std::string::npos);
    const auto stateReads = stateAuth.Calls;
    stateAuth.Allowed = false;
    assert(stateMonitor.ReadJson("Test.Serial.State") == State::StateDynamicReadStatus::UnsupportedFormat);
    assert(stateAuth.Calls == stateReads + 1);
    stateAuth.Allowed = true;
    assert(stateMonitor.ReadJson("Test.Serial.LocalState") == State::StateDynamicReadStatus::UnsupportedFormat);

    Serial::CommandConsole commandConsole;
    Serial::EventConsole eventConsole;
    assert(commandConsole.Initialize(console, directory.View(), commandAuth));
    assert(eventConsole.Initialize(console, directory.View(), eventAuth));

    stream.Clear();
    assert(console.ExecuteLine(R"(command json Test.Serial.Command {"value":7})") == Serial::ConsoleExecutionResult::Executed);
    assert(CommandSubmissions == 1);
    assert(stream.Output.find("Accepted") != std::string::npos);

    stream.Clear();
    assert(console.ExecuteLine(R"(command json Test.Serial.RequestCommand {"value":7})") == Serial::ConsoleExecutionResult::Executed);
    assert(CommandSubmissions == 1);
    assert(stream.Output.find("requester capability") != std::string::npos);

    commandAuth.Allowed = false;
    assert(console.ExecuteLine(R"(command json Test.Serial.Command {"value":7})") == Serial::ConsoleExecutionResult::Executed);
    assert(CommandSubmissions == 1);
    commandAuth.Allowed = true;

    stream.Clear();
    assert(console.ExecuteLine(R"(event json Test.Serial.Event {"value":8})") == Serial::ConsoleExecutionResult::Executed);
    assert(EventDispatches == 1);
    assert(stream.Output.find("Accepted") != std::string::npos);

    eventAuth.Allowed = false;
    assert(console.ExecuteLine(R"(event json Test.Serial.Event {"value":8})") == Serial::ConsoleExecutionResult::Executed);
    assert(EventDispatches == 1);

    commandConsole.Shutdown();
    eventConsole.Shutdown();
    stateMonitor.Shutdown();
    commandMonitor.Shutdown();
    eventMonitor.Shutdown();
    return 0;
}
