#include <Arduino.h>

#include <ESPressio_ArduinoByteStream.hpp>
#include <ESPressio_Event.hpp>
#include <ESPressio_TypeDirectory.hpp>

#include <ESPressio_Console.hpp>
#include <ESPressio_EventConsole.hpp>
#include <ESPressio_EventMonitor.hpp>

namespace E = ESPressio::Event;
using namespace ESPressio;

struct RemoteCommandEvent final : E::SerializableEvent<RemoteCommandEvent> {
    static constexpr E::EventTypeId TypeId{0x5103};
    static constexpr std::string_view CanonicalName =
        "flowduino.example.serial.remote-command.v1";
    static constexpr std::size_t MaximumLiveInstances = 4;
    static constexpr std::size_t MaximumPendingInstances = 1;

    std::int32_t Command = 0;
    std::int32_t Value = 0;

    ESPRESSIO_SERIALIZABLE_TYPE(RemoteCommandEvent)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("command", Command),
        ESPRESSIO_PROPERTY("value", Value)
    )
};

static_assert(Serializable::IsBoundedSerializable<RemoteCommandEvent>);

class LoopbackEventAuthorizer final : public Serial::IEventConsoleAuthorizer {
public:
    Serial::EventConsoleAuthorizationDecision Authorize(
        const Primitive::PrimitiveTypeDescriptor& descriptor
    ) const noexcept override {
        return descriptor.Key.Family == E::EventFamilyId &&
               descriptor.Key.TypeValue == RemoteCommandEvent::TypeId.Value()
            ? Serial::EventConsoleAuthorizationDecision::Authorized
            : Serial::EventConsoleAuthorizationDecision::Denied;
    }
};

Primitive::TypeDirectory<1> primitiveTypes;
E::Runtime events;
ESP32Platform::ArduinoByteStream serialIO(::Serial);
Serial::Console console;
Serial::EventConsole eventConsole;
Serial::EventMonitor eventMonitor;
LoopbackEventAuthorizer eventAuthorizer;

void setup() {
    ::Serial.begin(115200);

    // The historical transport loopback is intentionally replaced by a local
    // tooling loopback: Console parsing -> final Event family dispatch.
    if (primitiveTypes.Register<RemoteCommandEvent>() !=
        Primitive::TypeDirectoryRegistrationStatus::Success ||
        primitiveTypes.Initialize() !=
        Primitive::TypeDirectoryInitializationStatus::Success) {
        ::Serial.println("Failed to prepare Primitive TypeDirectory");
        return;
    }

    if (events.Initialize(primitiveTypes.View()) != E::EventRuntimeStatus::Success ||
        events.Start() != E::EventRuntimeStatus::Success) {
        ::Serial.println("Failed to start Event runtime");
        return;
    }

    Serial::ConsoleConfig consoleConfig;
    consoleConfig.Prompt = "espressio> ";
    consoleConfig.MaximumLineLength = 192;
    if (!console.Initialize(serialIO, consoleConfig)) {
        ::Serial.println("Failed to initialize Serial console");
        return;
    }

    Serial::EventConsoleConfig eventConsoleConfig;
    eventConsoleConfig.MaximumJsonLength = 128;
    if (!eventConsole.Initialize(
            console,
            primitiveTypes.View(),
            eventAuthorizer,
            eventConsoleConfig)) {
        ::Serial.println("Failed to initialize Event console");
        return;
    }

    auto* output = console.GetOutput();
    if (output == nullptr ||
        !eventMonitor.Initialize(primitiveTypes.View(), *output)) {
        ::Serial.println("Failed to initialize Event monitor");
        return;
    }

    eventMonitor.List();
    (void)eventMonitor.Describe(RemoteCommandEvent::CanonicalName);

    ::Serial.println();
    ::Serial.println("Local tooling loopback example:");
    ::Serial.println(
        "  event json flowduino.example.serial.remote-command.v1 {\"command\":1,\"value\":42}");

    // Exercise exactly the same parser/authorization/dynamic-dispatch path that
    // an operator enters on the serial console.
    (void)console.ExecuteLine(
        "event json flowduino.example.serial.remote-command.v1 {\"command\":1,\"value\":42}");
}

void loop() {
    console.Poll();
    delay(1);
}
