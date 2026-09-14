#pragma once

#if !__has_include(<ESPressio_Timing.hpp>)
#error "SystemClockMonitor requires the final ESPressio Timing observer contract."
#endif

#include <Arduino.h>
#include <cinttypes>
#include <cstdio>
#include <ESPressio_Timing.hpp>
#include <ESPressio_SystemClock.hpp>
#include <ESPressio_ISystemClockObserver.hpp>
#include <ESPressio_ClockSynchronization.hpp>
#include <ESPressio_TimeReliability.hpp>
#include <ESPressio_IObserver.hpp>

namespace ESPressio::Serial {

/// <summary>Writes final System Clock mutation, synchronization, reliability, and callback diagnostics to an Arduino Print sink.</summary>
template<typename TTick = ESPressio::Timing::ClockTick>
class SystemClockMonitor final : public ESPressio::Timing::ISystemClockObserver<TTick> {
private:
    Print* _output = nullptr;
    ESPressio::Observable::ObserverHandlePtr _handle;

    void Prefix(const char* operation) {
        if (!_output) return;
        _output->print("[ESPressio Timing] ");
        _output->print(operation);
    }

    void UnsignedValue(std::uint64_t value) {
        char buffer[32]{};
        std::snprintf(buffer, sizeof(buffer), "%" PRIu64, value);
        _output->print(buffer);
    }

    void SignedValue(std::int64_t value) {
        char buffer[32]{};
        std::snprintf(buffer, sizeof(buffer), "%" PRId64, value);
        _output->print(buffer);
    }

    void Difference(std::int64_t value) {
        _output->print(" diffNs=");
        SignedValue(value);
        _output->println();
    }

public:
    bool Initialize(
        Print& output,
        ESPressio::Timing::SystemClock<ESPressio::Timing::DefaultClockTime>& clock =
            ESPressio::Timing::SystemClock<ESPressio::Timing::DefaultClockTime>::GetInstance()) {
        if (_handle) return true;
        _output = &output;
        _handle = clock.RegisterObserver(this);
        if (!_handle) _output = nullptr;
        return static_cast<bool>(_handle);
    }

    void Shutdown() {
        _handle.reset();
        _output = nullptr;
    }

    bool IsInitialized() const noexcept { return static_cast<bool>(_handle); }

    void OnSystemClockTimeSet(TTick before, TTick after, std::int64_t diff) override {
        Prefix("TimeSet");
        _output->print(" beforeNs="); UnsignedValue(static_cast<std::uint64_t>(before));
        _output->print(" afterNs="); UnsignedValue(static_cast<std::uint64_t>(after));
        Difference(diff);
    }

    void OnSystemClockSynchronizationSampleAccepted(
        TTick before, TTick after, std::int64_t diff,
        const ESPressio::Timing::ClockSynchronizationResult&,
        const ESPressio::Timing::ClockSynchronizationStatus&) override {
        Prefix("SynchronizationSampleAccepted");
        _output->print(" beforeNs="); UnsignedValue(static_cast<std::uint64_t>(before));
        _output->print(" afterNs="); UnsignedValue(static_cast<std::uint64_t>(after));
        Difference(diff);
    }

    void OnSystemClockSynchronized(
        TTick before, TTick after, std::int64_t diff,
        const ESPressio::Timing::ClockSynchronizationResult&,
        const ESPressio::Timing::ClockSynchronizationStatus&) override {
        Prefix("Synchronized");
        _output->print(" beforeNs="); UnsignedValue(static_cast<std::uint64_t>(before));
        _output->print(" afterNs="); UnsignedValue(static_cast<std::uint64_t>(after));
        Difference(diff);
    }

    void OnSystemClockSynchronizationSampleRejected(
        const ESPressio::Timing::ClockSynchronizationResult&,
        const ESPressio::Timing::ClockSynchronizationStatus&) override {
        Prefix("SynchronizationSampleRejected");
        _output->println();
    }

    void OnSystemClockSynchronizationStateChanged(
        ESPressio::Timing::TimeReliability previous,
        ESPressio::Timing::TimeReliability current,
        const ESPressio::Timing::ClockSynchronizationStatus&) override {
        Prefix("SynchronizationStateChanged");
        _output->print(" previous="); _output->print(static_cast<int>(previous));
        _output->print(" current="); _output->println(static_cast<int>(current));
    }

    void OnSystemClockSynchronizationReset(
        const ESPressio::Timing::ClockSynchronizationStatus&,
        const ESPressio::Timing::ClockSynchronizationStatus&) override {
        Prefix("SynchronizationReset");
        _output->println();
    }

    void OnSystemClockSynchronizationConfigurationChanged(
        const ESPressio::Timing::ClockSynchronizationProfile&,
        const ESPressio::Timing::ClockSynchronizationProfile&) override {
        Prefix("SynchronizationConfigurationChanged");
        _output->println();
    }

    void OnSystemClockCallbackScheduled(TTick when) override {
        Prefix("CallbackScheduled");
        _output->print(" scheduledNs="); UnsignedValue(static_cast<std::uint64_t>(when));
        _output->println();
    }

    void OnSystemClockCallbackScheduleFailed(TTick when) override {
        Prefix("CallbackScheduleFailed");
        _output->print(" scheduledNs="); UnsignedValue(static_cast<std::uint64_t>(when));
        _output->println();
    }

    void OnSystemClockCallbackExecuted(TTick scheduled, TTick actual, std::int64_t diff) override {
        Prefix("CallbackExecuted");
        _output->print(" scheduledNs="); UnsignedValue(static_cast<std::uint64_t>(scheduled));
        _output->print(" actualNs="); UnsignedValue(static_cast<std::uint64_t>(actual));
        Difference(diff);
    }

    void OnSystemClockCallbackExecutionFailed(
        TTick scheduled, TTick actual, std::int64_t diff, std::exception_ptr) override {
        Prefix("CallbackExecutionFailed");
        _output->print(" scheduledNs="); UnsignedValue(static_cast<std::uint64_t>(scheduled));
        _output->print(" actualNs="); UnsignedValue(static_cast<std::uint64_t>(actual));
        Difference(diff);
    }

    void OnSystemClockCallbacksCleared(std::size_t count) override {
        Prefix("CallbacksCleared");
        _output->print(" count=");
        _output->println(static_cast<unsigned long>(count));
    }
};

} // namespace ESPressio::Serial
