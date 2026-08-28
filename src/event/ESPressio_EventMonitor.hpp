#pragma once

#if !__has_include(<ESPressio_EventTransport.hpp>)
#error "ESPressio EventMonitor requires ESPressio Event >= 5.6.2 < 6.0.0."
#endif

#if !__has_include(<ESPressio_BinaryArchiveTraversal.hpp>)
#error "ESPressio EventMonitor requires ESPressio Serializable >= 0.10.1 < 1.0.0."
#endif

#include <mutex>

#include <Arduino.h>
#include <ESPressio_EventTransport.hpp>

#include "../ESPressio_SerialTypes.hpp"
#include "ESPressio_EventMonitorFormatter.hpp"
#include "ESPressio_EventMonitorPayloadSafety.hpp"

namespace ESPressio::Serial {

/// <summary>Writes Event transport transactions and optional payload representations to an Arduino Print sink.</summary>
/// <remarks>Structured payload output traverses the serialized ESPB payload directly and applies the configured depth, node, collection, and string limits.</remarks>
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


    /// <summary>Registers the monitor with an Event transport manager and selects its output/configuration.</summary>
    /// <param name="output">Destination for formatted transaction diagnostics.</param>
    /// <param name="config">Visibility and payload-format limits.</param>
    /// <param name="manager">Transport manager to observe; defaults to the process-wide singleton.</param>
    /// <returns>True when the observer registration is active.</returns>
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


    /// <summary>Unregisters from the transport manager and releases monitor references.</summary>
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


    /// <summary>Reports whether the monitor is currently registered with a transport manager.</summary>
    bool GetIsInitialized() const {
        std::lock_guard<std::mutex>
            lock(_mutex);

        return _initialized;
    }


    /// <summary>Returns a copy of the current transaction visibility and payload-format configuration.</summary>
    EventMonitorConfig
    GetConfig() const {
        std::lock_guard<std::mutex>
            lock(_mutex);

        return _config;
    }


    /// <summary>Replaces the transaction visibility and payload-format configuration used by subsequent callbacks.</summary>
    void SetConfig(
        const EventMonitorConfig& config
    ) {
        std::lock_guard<std::mutex>
            lock(_mutex);

        _config = config;
    }


    /// <summary>Formats a visible Event transport transaction according to the active monitor configuration.</summary>
    /// <remarks>When structured payload traversal exceeds validation or configured limits, a bounded hexadecimal fallback is emitted instead.</remarks>
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

        if (
            _config.PayloadFormat !=
                EventMonitorPayloadFormat::Structured
        ) {
            EventMonitorFormatter::
                PrintTransaction(
                    *_output,
                    transaction,
                    _config
                );
            return;
        }

        // Print the transaction metadata through the established formatter, but
        // suppress its legacy tree-building structured payload path. The
        // payload itself is then traversed directly from ESPB bytes without
        // constructing a second SerializationNode tree.
        EventMonitorConfig metadataConfig =
            _config;
        metadataConfig.PayloadFormat =
            EventMonitorPayloadFormat::None;

        EventMonitorFormatter::
            PrintTransaction(
                *_output,
                transaction,
                metadataConfig
            );

        _output->print("  payload: ");

        if (
            PrintStructuredEventPayload(
                *_output,
                transaction.Payload,
                transaction.PayloadSize,
                _config
            )
        ) {
            _output->println();
            return;
        }

        _output->print(
            "<invalid-or-outside-monitor-limits> "
        );
        PrintEventPayloadHexFallback(
            *_output,
            transaction.Payload,
            transaction.PayloadSize,
            _config
        );
        _output->println();
    }
};

}
