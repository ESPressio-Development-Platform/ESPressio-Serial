#include <Arduino.h>

#include <ESPressio_ArduinoByteStream.hpp>
#include <ESPressio_Event.hpp>
#include <ESPressio_TypeDirectory.hpp>

#include <ESPressio_Console.hpp>
#include <ESPressio_EventConsole.hpp>

namespace E = ESPressio::Event;
using namespace ESPressio;

struct OperatorMessageEvent final : E::SerializableEvent<OperatorMessageEvent> {
    static constexpr E::EventTypeId TypeId{0x5101};
    static constexpr std::string_view CanonicalName =
        "flowduino.example.serial.operator-message.v1";
    static constexpr std::size_t MaximumLiveInstances = 4;
    static constexpr std::size_t MaximumPendingInstances = 1;

    std::int32_t Code = 0;
    std::uint32_t Sequence = 0;

    ESPRESSIO_SERIALIZABLE_TYPE(OperatorMessageEvent)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("code", Code),
        ESPRESSIO_PROPERTY("sequence", Sequence)
    )
};

static_assert(Serializable::IsBoundedSerializable<OperatorMessageEvent>);

class OperatorEventAuthorizer final : public Serial::IEventConsoleAuthorizer {
public:
    Serial::EventConsoleAuthorizationDecision Authorize(
        const Primitive::PrimitiveTypeDescriptor& descriptor
    ) const noexcept override {
        return descriptor.Key.Family == E::EventFamilyId &&
               descriptor.Key.TypeValue == OperatorMessageEvent::TypeId.Value()
            ? Serial::EventConsoleAuthorizationDecision::Authorized
            : Serial::EventConsoleAuthorizationDecision::Denied;
    }
};

Primitive::TypeDirectory<1> primitiveTypes;
E::Runtime events;
ESP32Platform::ArduinoByteStream serialIO(::Serial);
Serial::Console console;
Serial::EventConsole eventConsole;
OperatorEventAuthorizer eventAuthorizer;

void setup() {
    ::Serial.begin(115200);

    // The platform application must install its normal System execution,
    // synchronization and clock providers before Event runtime bootstrap.
    if (primitiveTypes.Register<OperatorMessageEvent>() !=
        Primitive::TypeDirectoryRegistrationStatus::Success) {
        ::Serial.println("Failed to register OperatorMessageEvent");
        return;
    }

    if (primitiveTypes.Initialize() !=
        Primitive::TypeDirectoryInitializationStatus::Success) {
        ::Serial.println("Failed to freeze Primitive TypeDirectory");
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

    ::Serial.println();
    ::Serial.println("Try:");
    ::Serial.println("  events");
    ::Serial.println(
        "  event describe flowduino.example.serial.operator-message.v1");
    ::Serial.println(
        "  event json flowduino.example.serial.operator-message.v1 {\"code\":7,\"sequence\":1}");
}

void loop() {
    console.Poll();
    delay(1);
}
