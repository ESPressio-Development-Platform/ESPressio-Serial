#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>

#include <ESPressio_EventConsole.hpp>
#include <ESPressio_EventTypeDescriptor.hpp>
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
    System::PlatformResult Write(const std::uint8_t* data, std::size_t size, std::size_t& written) noexcept override {
        written = 0;
        if (!data && size) return System::PlatformResult::Failed(System::PlatformStatus::InvalidArgument);
        try {
            if (size) Output.append(reinterpret_cast<const char*>(data), size);
            written = size;
            return System::PlatformResult::Succeeded();
        } catch (...) {
            return System::PlatformResult::Failed(System::PlatformStatus::OutOfMemory);
        }
    }
    void Clear() { Output.clear(); }
};

std::uint32_t Dispatches = 0;

Event::EventDynamicDispatchResult DispatchEvent(Event::EventPayloadFormat format, const std::uint8_t* payload, std::size_t size) noexcept {
    assert(format == Event::EventPayloadFormat::JSON);
    assert(std::string_view(reinterpret_cast<const char*>(payload), size) == R"({"value":42})");
    ++Dispatches;
    return {Event::EventDynamicDispatchStatus::Accepted, Primitive::ConceptualMessageId{1}};
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

Primitive::PrimitiveTypeDescriptor EventDescriptor() {
    static Event::EventTypeDescriptor extension{};
    extension.TypeId = Event::EventTypeId{0xB101};
    extension.Tier = Event::EventTier::Serializable;
    extension.MaximumLiveInstances = 2;
    extension.MaximumPendingInstances = 1;
    extension.Schema = &TestSchema();
    extension.DynamicallyConstructible = true;
    extension.DispatchSerialized = &DispatchEvent;
    return {{Event::EventFamilyId, extension.TypeId.Value()}, "Test.Serial.Event", Primitive::PrimitiveTypeCapabilities{1}, {1,1}, {}, {64}, {&extension}};
}

Primitive::PrimitiveTypeDescriptor StaticEventDescriptor() {
    static Event::EventTypeDescriptor extension{};
    extension.TypeId = Event::EventTypeId{0xB102};
    extension.Tier = Event::EventTier::Local;
    extension.MaximumLiveInstances = 1;
    extension.MaximumPendingInstances = 1;
    extension.Schema = &TestSchema();
    extension.DynamicallyConstructible = false;
    extension.DispatchSerialized = nullptr;
    return {{Event::EventFamilyId, extension.TypeId.Value()}, "Test.Serial.StaticEvent", Primitive::PrimitiveTypeCapabilities{}, {1,1}, {}, {0}, {&extension}};
}

class Authorizer final : public Serial::IEventConsoleAuthorizer {
public:
    bool Allowed = true;
    mutable std::uint32_t Calls = 0;
    Serial::EventConsoleAuthorizationDecision Authorize(const Primitive::PrimitiveTypeDescriptor&) const noexcept override {
        ++Calls;
        return Allowed ? Serial::EventConsoleAuthorizationDecision::Authorized : Serial::EventConsoleAuthorizationDecision::Denied;
    }
};

} // namespace

int main() {
    Primitive::TypeDirectory<2> directory;
    assert(directory.Register(EventDescriptor()) == Primitive::TypeDirectoryRegistrationStatus::Success);
    assert(directory.Register(StaticEventDescriptor()) == Primitive::TypeDirectoryRegistrationStatus::Success);
    assert(directory.Initialize() == Primitive::TypeDirectoryInitializationStatus::Success);

    TestStream stream;
    Serial::Console console;
    Serial::ConsoleConfig consoleConfig;
    consoleConfig.ShowPrompt = false;
    consoleConfig.MaximumLineLength = 256;
    assert(console.Initialize(stream, stream, consoleConfig));

    Authorizer authorizer;
    Serial::EventConsole eventConsole;
    Serial::EventConsoleConfig eventConfig;
    eventConfig.MaximumJsonLength = 64;
    assert(eventConsole.Initialize(console, directory.View(), authorizer, eventConfig));
    assert(eventConsole.GetIsInitialized());

    assert(console.ExecuteLine("events") == Serial::ConsoleExecutionResult::Executed);
    assert(stream.Output.find("Test.Serial.Event") != std::string::npos);
    assert(stream.Output.find("Test.Serial.StaticEvent") != std::string::npos);

    stream.Clear();
    assert(console.ExecuteLine("event describe Test.Serial.Event") == Serial::ConsoleExecutionResult::Executed);
    assert(stream.Output.find("JSON payload bytes: 32") != std::string::npos);
    assert(stream.Output.find("Dynamic construction: available") != std::string::npos);

    stream.Clear();
    assert(console.ExecuteLine(R"(event json Test.Serial.Event {"value":42})") == Serial::ConsoleExecutionResult::Executed);
    assert(Dispatches == 1);
    assert(stream.Output.find("Event dispatch: Accepted") != std::string::npos);

    authorizer.Allowed = false;
    stream.Clear();
    assert(console.ExecuteLine(R"(event json Test.Serial.Event {"value":42})") == Serial::ConsoleExecutionResult::Executed);
    assert(Dispatches == 1);
    assert(stream.Output.find("not authorized") != std::string::npos);
    authorizer.Allowed = true;

    stream.Clear();
    assert(console.ExecuteLine(R"(event json Test.Serial.StaticEvent {"value":42})") == Serial::ConsoleExecutionResult::Executed);
    assert(Dispatches == 1);
    assert(stream.Output.find("not dynamically constructible") != std::string::npos);

    stream.Clear();
    assert(console.ExecuteLine("event json Test.Serial.Event 123456789012345678901234567890123") == Serial::ConsoleExecutionResult::Executed);
    assert(Dispatches == 1);
    assert(stream.Output.find("exceeds bounded console/schema capacity") != std::string::npos);

    eventConsole.Shutdown();
    assert(!eventConsole.GetIsInitialized());
    return 0;
}
