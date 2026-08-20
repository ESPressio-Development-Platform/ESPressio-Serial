#pragma once

#if !__has_include(<ESPressio_EventTransport.hpp>)
#error "ESPressio EventMonitor requires ESPressio Event >= 5.6.2 < 6.0.0."
#endif

#if !__has_include(<ESPressio_BinaryArchive.hpp>)
#error "ESPressio EventMonitor requires ESPressio Serializable >= 0.10.1 < 1.0.0."
#endif

#include <mutex>

#include <Arduino.h>
#include <ESPressio_EventTransport.hpp>

#include "../ESPressio_SerialTypes.hpp"
#include "ESPressio_EventMonitorFormatter.hpp"
#include "ESPressio_EventMonitorPayloadSafety.hpp"

namespace ESPressio::Serial {

class EventMonitor final :
    public Event::IEventTransportManagerObserver {

private:
    Print* _output = nullptr;

    Event::EventTransportManager*
        _manager = nullptr;

    EventMonitorConfig _config;

    Observable::ObserverHandlePtr
        _observerHandle;

    mutable std::mutex _mutex;

    bool _initialized = false;


public:
    EventMonitor() = default;

    EventMonitor(
        const EventMonitor&
    ) = delete;

    EventMonitor&
    operator=(
        const EventMonitor&
    ) = delete;


    ~EventMonitor() override {
        Shutdown();
    }


    bool Initialize(
        Print& output,
        const EventMonitorConfig&
            config = {},
        Event::EventTransportManager&
            manager =
                Event::EventTransportManager::
                    GetInstance()
    ) {
        std::lock_guard<std::mutex>
            lock(_mutex);

        if (_initialized) {
            return true;
        }

        _output = &output;
        _config = config;
        _manager = &manager;

        _observerHandle =
            _manager->RegisterObserver(
                this
            );

        if (!_observerHandle) {
            _output = nullptr;
            _manager = nullptr;
            return false;
        }

        _initialized = true;
        return true;
    }


    void Shutdown() {
        Observable::ObserverHandlePtr
            handle;

        {
            std::lock_guard<std::mutex>
                lock(_mutex);

            if (!_initialized) {
                return;
            }

            handle =
                std::move(
                    _observerHandle
                );

            _initialized = false;
            _output = nullptr;
            _manager = nullptr;
        }

        handle.reset();
    }


    bool GetIsInitialized() const {
        std::lock_guard<std::mutex>
            lock(_mutex);

        return _initialized;
    }


    EventMonitorConfig
    GetConfig() const {
        std::lock_guard<std::mutex>
            lock(_mutex);

        return _config;
    }


    void SetConfig(
        const EventMonitorConfig& config
    ) {
        std::lock_guard<std::mutex>
            lock(_mutex);

        _config = config;
    }


    void OnEventTransportTransaction(
        const Event::EventTransportTransaction&
            transaction
    ) override {
        std::lock_guard<std::mutex>
            lock(_mutex);

        if (
            !_initialized ||
            _output == nullptr ||
            !EventMonitorFormatter::
                IsTransactionVisible(
                    transaction,
                    _config
                )
        ) {
            return;
        }

        EventMonitorConfig effectiveConfig =
            _config;

        if (
            effectiveConfig.PayloadFormat ==
                EventMonitorPayloadFormat::Structured &&
            transaction.Payload != nullptr &&
            transaction.PayloadSize != 0 &&
            !ValidateStructuredEventPayload(
                transaction.Payload,
                transaction.PayloadSize,
                effectiveConfig
            )
        ) {
            // Diagnostics must never make an invalid/unreasonable payload
            // fatal to the application. Preserve visibility with a bounded
            // hex fallback instead of attempting structured tree rendering.
            effectiveConfig.PayloadFormat =
                EventMonitorPayloadFormat::Hex;
        }

        EventMonitorFormatter::
            PrintTransaction(
                *_output,
                transaction,
                effectiveConfig
            );
    }
};

}
