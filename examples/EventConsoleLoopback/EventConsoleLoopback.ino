#include <Arduino.h>

#include <ESPressio_EventTransport.hpp>
#include <ESPressio_Event_Serializable.hpp>

#include <ESPressio_Console.hpp>
#include <ESPressio_EventConsole.hpp>
#include <ESPressio_EventMonitor.hpp>

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 24 bytes [0 bytes dynamic allocation]
 * Members:
 * - Command (String): 12 bytes [Capacity + 1 bytes backing buffer when allocated]
 * - Value (int32_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 40 bytes [Command: Capacity + 1 bytes backing buffer when allocated]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class RemoteCommandEvent final :
    public ESPressio::Event::
        SerializableEvent<
            RemoteCommandEvent
        > {

public:
    String Command;
    int32_t Value = 0;

    ESPRESSIO_SERIALIZABLE_TYPE(
        RemoteCommandEvent
    )

    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)

    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY(
            "command",
            Command
        ),
        ESPRESSIO_PROPERTY(
            "value",
            Value
        )
    )
};

ESPRESSIO_EVENT_TRANSPORT_TYPE(
    RemoteCommandEvent,
    "flowduino.example.serial.remote-command.v1"
)

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members:
 * - _receiver (ESPressio::Event::IEventTransportReceiver*): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 8 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class LoopbackTransport final :
    public ESPressio::Event::
        IEventTransport {

private:
    ESPressio::Event::
        IEventTransportReceiver*
            _receiver = nullptr;

public:
    bool Send(
        const ESPressio::Event::
            EventTransportPacket& packet
    ) override {
        if (
            _receiver == nullptr ||
            packet.Data == nullptr ||
            packet.Size == 0
        ) {
            return false;
        }

        _receiver->
            ReceiveEventTransportPacket(
                this,
                packet.Data,
                packet.Size
            );

        return true;
    }

    void SetReceiver(
        ESPressio::Event::
            IEventTransportReceiver*
                receiver
    ) override {
        _receiver = receiver;
    }
};

LoopbackTransport loopback;
ESPressio::Serial::Console console;
ESPressio::Serial::EventConsole eventConsole;
ESPressio::Serial::EventMonitor eventMonitor;

void setup() {
    ::Serial.begin(115200);

    auto& manager =
        ESPressio::Event::
            EventTransportManager::
                GetInstance();

    manager.RegisterTransport(
        &loopback
    );

    manager.RegisterBidirectionalEvent<
        RemoteCommandEvent
    >(
        &loopback
    );

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

    eventConsole.AllowEvent<
        RemoteCommandEvent
    >();

    ESPressio::Serial::
        EventMonitorConfig
            monitorConfig;

    monitorConfig.PayloadFormat =
        ESPressio::Serial::
            EventMonitorPayloadFormat::
                Structured;

    eventMonitor.Initialize(
        ::Serial,
        monitorConfig,
        manager
    );

    manager.Initialize();

    ::Serial.println();
    ::Serial.println(
        "Compose and dispatch a runtime Event over the loopback transport:"
    );

    ::Serial.println(
        "event queue flowduino.example.serial.remote-command.v1 {\"command\":\"move\",\"value\":42}"
    );
}

void loop() {
    console.Poll();
    delay(1);
}
