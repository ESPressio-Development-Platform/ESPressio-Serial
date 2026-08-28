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

#include <ESPressio_EventTransport.hpp>
#include <ESPressio_Memory.hpp>

#include "../console/ESPressio_Console.hpp"
#include "../ESPressio_SerialTypes.hpp"
#include "ESPressio_EventMonitorFormatter.hpp"
#include "ESPressio_EventMonitorPayloadSafety.hpp"

namespace ESPressio::Serial {

/// <summary>Writes Event transport transactions and optional payload representations to an ESPressio byte-output sink.</summary>
/// <remarks>Each diagnostic record is first composed into a bounded System <c>ExternalPreferred</c> buffer and then emitted with one bulk sink write. This keeps repeated formatting/output calls off the observed Event transport worker's constrained stack and greatly reduces cross-task output interleaving. Structured payload traversal remains bounded by the configured depth, node, collection, string, and rendered-byte limits.</remarks>
class EventMonitor final :
    public Event::IEventTransportManagerObserver {

private:
    using RenderBuffer = System::Memory::String<
        System::Memory::MemoryPolicy::ExternalPreferred
    >;

    /// <summary>Bounded byte sink used while composing one Event diagnostic record.</summary>
    class BufferedOutput final : public System::IO::IByteOutput {
    public:
        explicit BufferedOutput(std::size_t maximumBytes)
            : _maximumBytes(maximumBytes),
              _writer(this) {
            if (_maximumBytes != 0) {
                try {
                    _buffer.reserve(
                        std::min<std::size_t>(
                            _maximumBytes,
                            512
                        )
                    );
                } catch (...) {
                    _renderFailed = true;
                }
            }
        }

        /// <inheritdoc/>
        System::PlatformResult Write(
            const uint8_t* data,
            std::size_t size,
            std::size_t& bytesWritten
        ) noexcept override {
            bytesWritten = 0;

            if (data == nullptr && size != 0) {
                return System::PlatformResult::Failed(
                    System::PlatformStatus::InvalidArgument
                );
            }

            if (size == 0) {
                return System::PlatformResult::Succeeded();
            }

            // Formatting is best-effort diagnostics. Once the configured bound
            // has been reached, consume the remaining logical writes without
            // extending the buffer so callers do not repeatedly retry them.
            if (
                _maximumBytes == 0 ||
                _buffer.size() >= _maximumBytes ||
                _renderFailed
            ) {
                _truncated = true;
                bytesWritten = size;
                return System::PlatformResult::Succeeded();
            }

            const std::size_t writable =
                std::min(
                    size,
                    _maximumBytes - _buffer.size()
                );

            try {
                _buffer.append(
                    reinterpret_cast<const char*>(data),
                    writable
                );
            } catch (...) {
                _renderFailed = true;
                _truncated = true;
                bytesWritten = size;
                return System::PlatformResult::Succeeded();
            }

            if (writable != size) {
                _truncated = true;
            }

            bytesWritten = size;
            return System::PlatformResult::Succeeded();
        }

        /// <summary>Returns the text writer used by existing diagnostic formatters.</summary>
        Print& Writer() noexcept {
            return _writer;
        }

        /// <summary>Returns the rendered bytes accumulated for the current record.</summary>
        const RenderBuffer& Buffer() const noexcept {
            return _buffer;
        }

        /// <summary>Indicates that rendering exceeded its configured byte limit.</summary>
        bool Truncated() const noexcept {
            return _truncated;
        }

        /// <summary>Indicates that the backing diagnostic buffer could not be extended.</summary>
        bool RenderFailed() const noexcept {
            return _renderFailed;
        }

    private:
        RenderBuffer _buffer;
        std::size_t _maximumBytes = 0;
        ByteOutputTextWriter _writer;
        bool _truncated = false;
        bool _renderFailed = false;
    };

    Print* _output = nullptr;

    Event::EventTransportManager*
        _manager = nullptr;

    EventMonitorConfig _config;

    Observable::ObserverHandlePtr
        _observerHandle;

    mutable std::mutex _mutex;

    bool _initialized = false;

    void FlushRecord(BufferedOutput& rendered) noexcept {
        if (_output == nullptr) return;

        const auto& buffer = rendered.Buffer();
        if (!buffer.empty()) {
            (void)_output->write(
                reinterpret_cast<const uint8_t*>(buffer.data()),
                buffer.size()
            );
        }

        if (rendered.RenderFailed()) {
            static constexpr char marker[] =
                "\n  <diagnostic-render-failed>\n";
            (void)_output->write(
                reinterpret_cast<const uint8_t*>(marker),
                sizeof(marker) - 1
            );
        } else if (rendered.Truncated()) {
            static constexpr char marker[] =
                "\n  <diagnostic-output-truncated>\n";
            (void)_output->write(
                reinterpret_cast<const uint8_t*>(marker),
                sizeof(marker) - 1
            );
        }
    }

    void RenderTransaction(
        BufferedOutput& rendered,
        const Event::EventTransportTransaction& transaction
    ) {
        Print& writer = rendered.Writer();

        if (
            _config.PayloadFormat !=
                EventMonitorPayloadFormat::Structured
        ) {
            EventMonitorFormatter::PrintTransaction(
                writer,
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
            writer,
            transaction,
            metadataConfig
        );

        writer.print("  payload: ");

        if (
            PrintStructuredEventPayload(
                writer,
                transaction.Payload,
                transaction.PayloadSize,
                _config
            )
        ) {
            writer.println();
            return;
        }

        writer.print("<invalid-or-outside-monitor-limits> ");
        PrintEventPayloadHexFallback(
            writer,
            transaction.Payload,
            transaction.PayloadSize,
            _config
        );
        writer.println();
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
            BufferedOutput rendered(
                _config.MaximumRenderedBytes
            );
            RenderTransaction(
                rendered,
                transaction
            );
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
