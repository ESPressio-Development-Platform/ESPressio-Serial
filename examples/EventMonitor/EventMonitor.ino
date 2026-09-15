#include <Arduino.h>

#include <ESPressio_ArduinoByteStream.hpp>
#include <ESPressio_Event.hpp>
#include <ESPressio_TypeDirectory.hpp>

#include <ESPressio_Console.hpp>
#include <ESPressio_EventMonitor.hpp>

namespace E = ESPressio::Event;
using namespace ESPressio;

struct MonitoredCounterEvent final : E::SerializableEvent<MonitoredCounterEvent> {
    static constexpr E::EventTypeId TypeId{0x5102};
    static constexpr std::string_view CanonicalName =
        "flowduino.example.serial.monitored-counter.v1";
    static constexpr std::size_t MaximumLiveInstances = 4;
    static constexpr std::size_t MaximumPendingInstances = 1;

    std::uint32_t Counter = 0;
    std::uint32_t Source = 0;

    ESPRESSIO_SERIALIZABLE_TYPE(MonitoredCounterEvent)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("counter", Counter),
        ESPRESSIO_PROPERTY("source", Source)
    )
};

static_assert(Serializable::IsBoundedSerializable<MonitoredCounterEvent>);

Primitive::TypeDirectory<1> primitiveTypes;
ESP32Platform::ArduinoByteStream serialIO(::Serial);
Serial::Console console;
Serial::EventMonitor eventMonitor;

void setup() {
    ::Serial.begin(115200);

    if (primitiveTypes.Register<MonitoredCounterEvent>() !=
        Primitive::TypeDirectoryRegistrationStatus::Success) {
        ::Serial.println("Failed to register MonitoredCounterEvent");
        return;
    }

    if (primitiveTypes.Initialize() !=
        Primitive::TypeDirectoryInitializationStatus::Success) {
        ::Serial.println("Failed to freeze Primitive TypeDirectory");
        return;
    }

    Serial::ConsoleConfig consoleConfig;
    consoleConfig.ShowPrompt = false;

    if (!console.Initialize(serialIO, consoleConfig)) {
        ::Serial.println("Failed to initialize Serial console");
        return;
    }

    auto* output = console.GetOutput();
    if (output == nullptr ||
        !eventMonitor.Initialize(primitiveTypes.View(), *output)) {
        ::Serial.println("Failed to initialize Event monitor");
        return;
    }

    // Final Event diagnostics are descriptor/read-only tooling. They do not
    // subscribe to a transport manager or own Event delivery lifecycle state.
    eventMonitor.List();
    (void)eventMonitor.Describe(MonitoredCounterEvent::CanonicalName);
}

void loop() {
    delay(1000);
}
