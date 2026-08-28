#pragma once

#if !__has_include(<ESPressio_EventTransport.hpp>)
#error "ESPressio EventMonitor requires ESPressio Event transport support."
#endif

#if !__has_include(<ESPressio_BinaryArchiveTraversal.hpp>)
#error "ESPressio EventMonitor requires ESPressio Serializable BinaryArchive traversal support."
#endif

#if !__has_include(<ESPressio_Task.hpp>)
#error "ESPressio EventMonitor requires ESPressio Task support."
#endif

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <new>
#include <string_view>
#include <type_traits>
#include <utility>

#include <ESPressio_EventTransport.hpp>
#include <ESPressio_Memory.hpp>
#include <ESPressio_Task.hpp>

#include "../console/ESPressio_Console.hpp"
#include "../ESPressio_SerialTypes.hpp"
#include "ESPressio_EventMonitorFormatter.hpp"
#include "ESPressio_EventMonitorPayloadSafety.hpp"

namespace ESPressio::Serial {

/// <summary>Writes Event transport transactions and optional payload representations to an ESPressio byte-output sink.</summary>
/// <remarks>
/// Event transport callbacks only capture a bounded, owned transaction snapshot into System
/// <c>ExternalPreferred</c> memory and submit a pointer to a dedicated bounded diagnostic worker.
/// Formatting, BinaryArchive traversal, numeric conversion, and output therefore never execute on
/// the observed Event transport worker's constrained stack.
/// </remarks>
class EventMonitor final :
    public Event::IEventTransportManagerObserver {

private:
    using RenderBuffer = System::Memory::String<
        System::Memory::MemoryPolicy::ExternalPreferred
    >;

    using SnapshotString = System::Memory::String<
        System::Memory::MemoryPolicy::ExternalPreferred
    >;

    using SnapshotBytes = System::Memory::Vector<
        uint8_t,
        System::Memory::MemoryPolicy::ExternalPreferred
    >;

    static constexpr uint32_t DiagnosticWorkerStackSize = 4096;
    static constexpr std::size_t DiagnosticQueueDepth = 4;

    /// <summary>Owns every transient view required to render a transaction after its originating callback returns.</summary>
    struct TransactionSnapshot {
        System::Memory::IMemoryProvider* Provider = nullptr;
        Event::EventTransportTransaction Transaction{};
        SnapshotString EventTypeName;
        SnapshotBytes Payload;
        EventMonitorConfig Config{};
        Print* Output = nullptr;

        TransactionSnapshot(
            System::Memory::IMemoryProvider& provider,
            const Event::EventTransportTransaction& transaction,
            const EventMonitorConfig& config,
            Print* output
        ) :
            Provider(&provider),
            Transaction(transaction),
            Config(config),
            Output(output) {

            if (!transaction.EventTypeName.empty()) {
                EventTypeName.assign(
                    transaction.EventTypeName.data(),
                    transaction.EventTypeName.size()
                );
            }

            if (
                transaction.Payload != nullptr &&
                transaction.PayloadSize != 0
            ) {
                Payload.assign(
                    transaction.Payload,
                    transaction.Payload + transaction.PayloadSize
                );
            }

            Transaction.EventTypeName = EventTypeName;
            Transaction.Payload =
                Payload.empty()
                    ? nullptr
                    : Payload.data();
            Transaction.PayloadSize = Payload.size();
        }
    };

    /// <summary>Trivially-copyable queue item used by the bounded TaskExecutor.</summary>
    struct DiagnosticWorkItem {
        TransactionSnapshot* Snapshot = nullptr;
    };

    static_assert(
        std::is_trivially_copyable<DiagnosticWorkItem>::value,
        "EventMonitor diagnostic work items must remain trivially copyable"
    );

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

    static Task::TaskConfiguration MakeWorkerConfiguration() noexcept {
        Task::TaskConfiguration configuration;
        configuration.Name = "EventDiagnostic";
        configuration.StackSize = DiagnosticWorkerStackSize;
        configuration.Priority = 1;
        configuration.Core = -1;
        configuration.QueueDepth = DiagnosticQueueDepth;
        configuration.OverflowPolicy =
            Task::TaskQueueOverflowPolicy::Reject;
        configuration.MemoryPolicy =
            Task::TaskMemoryPolicy::Internal;
        return configuration;
    }

    Print* _output = nullptr;

    Event::EventTransportManager*
        _manager = nullptr;

    EventMonitorConfig _config;

    Observable::ObserverHandlePtr
        _observerHandle;

    Task::TaskExecutor<DiagnosticWorkItem>
        _worker{MakeWorkerConfiguration()};

    mutable std::mutex _mutex;

    bool _initialized = false;

    static TransactionSnapshot* CreateSnapshot(
        const Event::EventTransportTransaction& transaction,
        const EventMonitorConfig& config,
        Print* output
    ) noexcept {
        auto& provider =
            System::Memory::GetProvider();

        void* storage = nullptr;

        try {
            storage = provider.Allocate(
                sizeof(TransactionSnapshot),
                alignof(TransactionSnapshot),
                System::Memory::MemoryPolicy::ExternalPreferred
            );

            if (storage == nullptr) {
                return nullptr;
            }

            return new (storage) TransactionSnapshot(
                provider,
                transaction,
                config,
                output
            );
        } catch (...) {
            if (storage != nullptr) {
                provider.Deallocate(
                    storage,
                    sizeof(TransactionSnapshot),
                    alignof(TransactionSnapshot),
                    System::Memory::MemoryPolicy::ExternalPreferred
                );
            }

            return nullptr;
        }
    }

    static void DestroySnapshot(
        TransactionSnapshot* snapshot
    ) noexcept {
        if (snapshot == nullptr) {
            return;
        }

        auto* provider = snapshot->Provider;

        snapshot->~TransactionSnapshot();

        if (provider != nullptr) {
            provider->Deallocate(
                snapshot,
                sizeof(TransactionSnapshot),
                alignof(TransactionSnapshot),
                System::Memory::MemoryPolicy::ExternalPreferred
            );
        }
    }

    static void WriteFallback(
        Print* output,
        const char* text
    ) noexcept {
        if (output == nullptr || text == nullptr) {
            return;
        }

        output->print(text);
    }

    static void FlushRecord(
        Print* output,
        BufferedOutput& rendered
    ) noexcept {
        if (output == nullptr) {
            return;
        }

        const auto& buffer = rendered.Buffer();

        if (!buffer.empty()) {
            (void)output->write(
                reinterpret_cast<const uint8_t*>(
                    buffer.data()
                ),
                buffer.size()
            );
        }

        if (rendered.RenderFailed()) {
            static constexpr char marker[] =
                "\n  <diagnostic-render-failed>\n";

            (void)output->write(
                reinterpret_cast<const uint8_t*>(
                    marker
                ),
                sizeof(marker) - 1
            );
        } else if (rendered.Truncated()) {
            static constexpr char marker[] =
                "\n  <diagnostic-output-truncated>\n";

            (void)output->write(
                reinterpret_cast<const uint8_t*>(
                    marker
                ),
                sizeof(marker) - 1
            );
        }
    }

    static void RenderTransaction(
        BufferedOutput& rendered,
        const Event::EventTransportTransaction& transaction,
        const EventMonitorConfig& config
    ) {
        Print& writer =
            rendered.Writer();

        if (
            config.PayloadFormat !=
                EventMonitorPayloadFormat::Structured
        ) {
            EventMonitorFormatter::PrintTransaction(
                writer,
                transaction,
                config
            );

            return;
        }

        EventMonitorConfig metadataConfig =
            config;

        metadataConfig.PayloadFormat =
            EventMonitorPayloadFormat::None;

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
                config
            )
        ) {
            writer.println();
            return;
        }

        writer.print(
            "<invalid-or-outside-monitor-limits> "
        );

        PrintEventPayloadHexFallback(
            writer,
            transaction.Payload,
            transaction.PayloadSize,
            config
        );

        writer.println();
    }

    static void ProcessWorkItem(
        const DiagnosticWorkItem& item
    ) noexcept {
        TransactionSnapshot* snapshot =
            item.Snapshot;

        if (snapshot == nullptr) {
            return;
        }

        Print* output =
            snapshot->Output;

        try {
            BufferedOutput rendered(
                snapshot->Config.MaximumRenderedBytes
            );

            RenderTransaction(
                rendered,
                snapshot->Transaction,
                snapshot->Config
            );

            FlushRecord(
                output,
                rendered
            );
        } catch (...) {
            WriteFallback(
                output,
                "[ESPressio Event] <diagnostic-render-failed>\n"
            );
        }

        DestroySnapshot(snapshot);
    }

    void DrainWorker() noexcept {
        for (;;) {
            const auto statistics =
                _worker.GetStatistics();

            if (
                statistics.Completed >=
                statistics.Submitted
            ) {
                return;
            }

            Task::TaskRuntime::SleepMilliseconds(1);
        }
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

    /// <summary>Registers the monitor with an Event transport manager and starts its isolated diagnostic worker.</summary>
    /// <param name="output">Destination for formatted transaction diagnostics.</param>
    /// <param name="config">Visibility and payload-format limits.</param>
    /// <param name="manager">Transport manager to observe; defaults to the process-wide singleton.</param>
    /// <returns>True when both the diagnostic worker and observer registration are active.</returns>
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

        const auto initialization =
            _worker.Initialize(
                [](const DiagnosticWorkItem& item) {
                    ProcessWorkItem(item);
                }
            );

        if (
            initialization !=
            Task::TaskExecutionStatus::Success
        ) {
            return false;
        }

        if (
            _worker.Start() !=
            Task::TaskExecutionStatus::Success
        ) {
            _worker.Stop();
            return false;
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
            _worker.Stop();
            return false;
        }

        _initialized = true;
        return true;
    }

    /// <summary>Stops observing, drains accepted diagnostic snapshots, and releases the isolated worker.</summary>
    void Shutdown() {
        Observable::ObserverHandlePtr
            handle;

        {
            std::lock_guard<std::mutex>
                lock(_mutex);

            if (!_initialized) {
                return;
            }

            _initialized = false;

            handle =
                std::move(
                    _observerHandle
                );
        }

        handle.reset();

        DrainWorker();

        _worker.Stop();

        {
            std::lock_guard<std::mutex>
                lock(_mutex);

            _output = nullptr;
            _manager = nullptr;
        }
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

    /// <summary>Replaces the transaction visibility and payload-format configuration copied into subsequent diagnostic snapshots.</summary>
    void SetConfig(
        const EventMonitorConfig& config
    ) {
        std::lock_guard<std::mutex>
            lock(_mutex);

        _config = config;
    }

    /// <summary>Captures a bounded owned transaction snapshot and queues it for diagnostic rendering outside the Event transport worker.</summary>
    /// <remarks>
    /// The callback performs no BinaryArchive traversal, numeric formatting, or Serial output. Snapshot
    /// storage uses the ESPressio System <c>ExternalPreferred</c> memory policy. When the bounded
    /// diagnostic queue is full or snapshot allocation fails, that diagnostic record is dropped rather
    /// than blocking or destabilizing the observed transport path.
    /// </remarks>
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

        TransactionSnapshot* snapshot =
            CreateSnapshot(
                transaction,
                _config,
                _output
            );

        if (snapshot == nullptr) {
            return;
        }

        DiagnosticWorkItem item;
        item.Snapshot = snapshot;

        if (
            _worker.Submit(item) !=
            Task::TaskExecutionStatus::Success
        ) {
            DestroySnapshot(snapshot);
        }
    }
};

}
