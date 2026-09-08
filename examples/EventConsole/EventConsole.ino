#include <Arduino.h>

#include <ESPressio_EventTransport.hpp>
#include <ESPressio_Event_Serializable.hpp>

#include <ESPressio_Console.hpp>
#include <ESPressio_EventConsole.hpp>

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 24 bytes [0 bytes dynamic allocation]
 * Members:
 * - Message (String): 12 bytes [Capacity + 1 bytes backing buffer when allocated]
 * - Sequence (uint32_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 40 bytes [Message: Capacity + 1 bytes backing buffer when allocated]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class OperatorMessageEvent final :
    public ESPressio::Event::
        SerializableEvent<
            OperatorMessageEvent
        > {

public:
    String Message;
    uint32_t Sequence = 0;

    ESPRESSIO_SERIALIZABLE_TYPE(
        OperatorMessageEvent
    )

    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)

    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY(
            "message",
            Message
        ),
        ESPRESSIO_PROPERTY(
            "sequence",
            Sequence
        )
    )
};

ESPRESSIO_EVENT_TRANSPORT_TYPE(
    OperatorMessageEvent,
    "flowduino.example.serial.operator-message.v1"
)

ESPressio::Serial::Console console;
ESPressio::Serial::EventConsole eventConsole;

void setup() {
    ::Serial.begin(115200);

    auto& manager =
        ESPressio::Event::
            EventTransportManager::
                GetInstance();

    /*
     * Registration places the type in Event 5.6's runtime Serializable
     * Event registry. Existing Event Transport routing rules still apply.
     */
    manager.RegisterBidirectionalEvent<
        OperatorMessageEvent
    >();

    ESPressio::Serial::ConsoleConfig
        consoleConfig;

    consoleConfig.Prompt =
        "espressio> ";

    console.Initialize(
        ::Serial,
        ::Serial,
        consoleConfig
    );

    ESPressio::Serial::
        EventConsoleConfig
            eventConsoleConfig;

    eventConsoleConfig.RequireConfirmation =
        true;

    eventConsole.Initialize(
        console,
        eventConsoleConfig,
        manager
    );

    /*
     * Safe default: only explicitly allowed Event types can be dispatched
     * by the operator.
     */
    eventConsole.AllowEvent<
        OperatorMessageEvent
    >();

    manager.Initialize();

    ::Serial.println();
    ::Serial.println("Try:");
    ::Serial.println("  events");
    ::Serial.println(
        "  event describe flowduino.example.serial.operator-message.v1"
    );
    ::Serial.println(
        "  event queue flowduino.example.serial.operator-message.v1 {\"message\":\"hello\",\"sequence\":1}"
    );
    ::Serial.println(
        "  event compose flowduino.example.serial.operator-message.v1"
    );
}

void loop() {
    console.Poll();
    delay(1);
}
