#include <Arduino.h>

#include <ESPressio_EventTransport.hpp>
#include <ESPressio_Event_Serializable.hpp>

#include <ESPressio_Console.hpp>
#include <ESPressio_EventConsole.hpp>

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
