#pragma once

#if !__has_include(<ESPressio_EventTransport.hpp>)
#error "ESPressio EventMonitor requires ESPressio Event >= 5.6.2 < 6.0.0."
#endif

#if !__has_include(<ESPressio_BinaryArchiveTraversal.hpp>)
#error "ESPressio EventMonitor requires ESPressio Serializable >= 0.10.1 < 1.0.0."
#endif

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <mutex>

#include <Arduino.h>
#include <ESPressio_EventTransport.hpp>
#include <ESPressio_Memory.hpp>

#include "../ESPressio_SerialTypes.hpp"
#include "ESPressio_EventMonitorFormatter.hpp"
#include "ESPressio_EventMonitorPayloadSafety.hpp"

namespace ESPressio::Serial {

/// <summary>Writes Event transport transactions and optional payload representations to an Arduino Print sink.</summary>
/// <remarks>Each diagnostic record is first composed into a bounded System <c>ExternalPreferred</c> buffer and then emitted with one bulk sink write. This keeps repeated Arduino Print/driver calls off the observed Event transport worker's constrained stack and greatly reduces cross-task output interleaving. Structured payload traversal remains bounded by the configured depth, node, collection, string, and rendered-byte limits.</remarks>
class EventMonitor final :
    public Event::IEventTransportManagerObserver {

private:
    using RenderBuffer = System::Memory::String<
        System::Memory::MemoryPolicy::ExternalPreferred
    >;

    class BufferedPrint final : public Print {
    public:
        explicit BufferedPrint(std::size_t maximumBytes)
            : _maximumBytes(maximumBytes) {
            if (_maximumBytes != 0) {
                _buffer.reserve(std::min<std::size_t>(_maximumBytes, 512));
            }
        }

        std::size_t write(uint8_t value) override {
            return write(&value, 1);
        }

        std::size_t write(
            const uint8_t* buffer,
            std::size_t size
        ) override {
            if (buffer == nullptr || size == 0) return 0;
            if (_maximumBytes == 0 || _buffer.size() >= _maximumBytes) {
                _truncated = true;
                return size;
            }

            const std::size_t writable = std::min(
                size,
                _maximumBytes - _buffer.size()
            );
            _buffer.append(
                reinterpret_cast<const char*>(buffer),
                writable
            );
            if (writable != size) _truncated = true;

            // From Print's perspective the bytes have been consumed. The
            // bounded renderer deliberately discards only the overflow tail.
            return size;
        }

        const RenderBuffer& Buffer() const noexcept { return _buffer; }
        bool Truncated() const noexcept { return _truncated; }

    private:
        RenderBuffer _buffer;
        std::size_t _maximumBytes = 0;
        bool _truncated = false;
    };

    Print* _output = nullptr;

    Event::EventTransportManager*
        _manager = nullptr;

    EventMonitorConfig _config;

    Observable::ObserverHandlePtr
        _observerHandle;

    mutable std::mutex _mutex;

    bool _initialized = false;

    void FlushRecord(BufferedPrint& rendered) noexcept {
        if (_output == nullptr) return;

        const auto& buffer = rendered.Buffer();
        if (!buffer.empty()) {
            (void)_output->write(
                reinterpret_cast<const uint8_t*>(buffer.data()),
                buffer.size()
            );
        }
        if (rendered.Truncated()) {
            static constexpr char marker[] = "\n  <diagnostic-output-truncated>\n";
            (void)_output->write(
                reinterpret_cast<const uint8_t*>(marker),
                sizeof(marker) - 1
            );
        }
    }

    void RenderTransaction(
        BufferedPrint& rendered,
        const Event::EventTransportTransaction& transaction
    ) {
        if (
            _config.PayloadFormat !=
                EventMonitorPayloadFormat::Structured
        ) {
            EventMonitorFormatter::PrintTransaction(
                rendered,
                transaction,
                _config
            );
            return;
        }

        // Print metadata through the established formatter, but suppress its
        // legacy tree-building structured payload path. The payload itself is
        // traversed directly from ESPB bytes without constructing a second DOM.
        EventMonitorConfig metadataConfig = _config;
        metadataConfig.PayloadFormat = EventMonitorPayloadFormat::None;

        EventMonitorFormatter::PrintTransaction(
            rendered,
            transaction,
            metadataConfig
        );

        rendered.print("  payload: ");

        if (
            PrintStructuredEventPayload(
                rendered,
                transaction.Payload,
                transaction.PayloadSize,
                _config
            )
        ) {
            rendered.println();
            return;
        }

        rendered.print("<invalid-or-outside-monitor-limits> ");
        PrintEventPayloadHexFallback(
            rendered,
            transaction.Payload,
            transaction.PayloadSize,
            _config
        );
        rendered.println();
    }

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

    /// <summary>Formats a visible Event transport transaction into external working memory and flushes it to the selected sink as one bounded record.</summary>
    /// <remarks>Rendering failures are contained inside the diagnostic observer so optional monitoring cannot terminate or corrupt the observed transport worker.</remarks>
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

        try {
            BufferedPrint rendered(_config.MaximumRenderedBytes);
            RenderTransaction(rendered, transaction);
            FlushRecord(rendered);
        } catch (...) {
            // Diagnostics must never be capable of failing the Event transport
            // path they observe. Keep the fallback deliberately small and free
            // of structured traversal/allocation.
            static constexpr char failure[] =
                "[ESPressio Event] <diagnostic-render-failed>\n";
            (void)_output->write(
                reinterpret_cast<const uint8_t*>(failure),
                sizeof(failure) - 1
            );
        }
    }
};

}
