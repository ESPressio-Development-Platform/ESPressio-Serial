#include <Arduino.h>

#include <ESPressio_EventTransport.hpp>
#include <ESPressio_Event_Serializable.hpp>
#include <ESPressio_EventMonitor.hpp>


class MonitoredCounterEvent final :
    public ESPressio::Event::
        SerializableEvent<
            MonitoredCounterEvent
        > {

public:
    uint32_t Counter = 0;
    String Source = "local";

    ESPRESSIO_SERIALIZABLE_TYPE(
        MonitoredCounterEvent
    )

    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)

    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY(
            "counter",
            Counter
        ),
        ESPRESSIO_PROPERTY(
            "source",
            Source
        )
    )
};


ESPRESSIO_EVENT_TRANSPORT_TYPE(
    MonitoredCounterEvent,
    "flowduino.example.serial.monitored-counter.v1"
)


/*
 * A tiny local loopback transport lets the example demonstrate both
 * outbound and inbound Event Transport monitoring without requiring
 * additional hardware or a network connection.
 */
class LoopbackEventTransport final :
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


LoopbackEventTransport
    loopbackTransport;

ESPressio::Serial::EventMonitor
    eventMonitor;


void setup() {
    ::Serial.begin(115200);

    delay(500);

    auto& manager =
        ESPressio::Event::
            EventTransportManager::
                GetInstance();

    manager.RegisterTransport(
        &loopbackTransport
    );

    manager.RegisterBidirectionalEvent<
        MonitoredCounterEvent
    >(
        &loopbackTransport
    );


    ESPressio::Serial::
        EventMonitorConfig
            monitorConfig;

    /*
     * Default mode prints one useful record per outbound/inbound Event.
     *
     * Use Lifecycle to print every Event 5.5 transaction stage:
     *
     * monitorConfig.Mode =
     *     ESPressio::Serial::
     *         EventMonitorMode::Lifecycle;
     */
    monitorConfig.PayloadFormat =
        ESPressio::Serial::
            EventMonitorPayloadFormat::
                Structured;

    eventMonitor.Initialize(
        ::Serial,
        monitorConfig,
        manager
    );


    const auto status =
        manager.Initialize();

    ::Serial.print(
        "EventTransportManager initialization status: "
    );

    ::Serial.println(
        static_cast<int>(
            status
        )
    );
}


void loop() {
    static uint32_t counter = 0;
    static uint32_t lastDispatch = 0;

    const uint32_t now =
        millis();

    if (
        now - lastDispatch >=
        5000
    ) {
        lastDispatch = now;

        auto* event =
            new MonitoredCounterEvent();

        event->Counter =
            ++counter;

        event->Queue();
    }

    delay(10);
}
