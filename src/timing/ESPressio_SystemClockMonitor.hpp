#pragma once
#if !__has_include(<ESPressio_Timing.hpp>)
#error "SystemClockMonitor requires ESPressio Timing >= 2.2.0."
#endif
#include <Arduino.h>
#include <cstdio>
#include <cinttypes>
#include <ESPressio_Timing.hpp>
#include <ESPressio_SystemClock.hpp>
#include <ESPressio_ISystemClockObserver.hpp>
#include <ESPressio_ClockSynchronization.hpp>
#include <ESPressio_IObserver.hpp>

namespace ESPressio::Serial {

template<typename TTick = ESPressio::Timing::ClockTick>
class SystemClockMonitor final :
    public ESPressio::Timing::ISystemClockObserver<TTick> {
    Print* _output = nullptr;
    ESPressio::Observable::ObserverHandlePtr _handle;

    void Prefix(const char* operation) {
        if (!_output) return;
        _output->print("[ESPressio Timing] ");
        _output->print(operation);
    }
    void UnsignedValue(uint64_t value) {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%" PRIu64, value);
        _output->print(buffer);
    }
    void SignedValue(int64_t value) {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%" PRId64, value);
        _output->print(buffer);
    }
    void Difference(int64_t value) {
        _output->print(" diffNs=");
        SignedValue(value);
        _output->println();
    }

public:
    bool Initialize(
        Print& output,
        ESPressio::Timing::SystemClock<ESPressio::Timing::DefaultClockTime>& clock =
            ESPressio::Timing::SystemClock<ESPressio::Timing::DefaultClockTime>::GetInstance()
    ) {
        if (_handle) return true;
        _output = &output;
        _handle = clock.RegisterObserver(this);
        if (!_handle) _output = nullptr;
        return static_cast<bool>(_handle);
    }

    void Shutdown() { _handle.reset(); _output = nullptr; }

    void OnSystemClockTimeSet(TTick before, TTick after, int64_t diff) override {
        Prefix("TimeSet");
        _output->print(" beforeNs="); UnsignedValue(static_cast<uint64_t>(before));
        _output->print(" afterNs="); UnsignedValue(static_cast<uint64_t>(after));
        Difference(diff);
    }

    void OnSystemClockSynchronizationSampleAccepted(
        TTick before, TTick after, int64_t diff,
        const ESPressio::Timing::ClockSynchronizationResult<TTick>&,
        const ESPressio::Timing::ClockSynchronizationStatus<TTick>&
    ) override {
        Prefix("SynchronizationSampleAccepted");
        _output->print(" beforeNs="); UnsignedValue(static_cast<uint64_t>(before));
        _output->print(" afterNs="); UnsignedValue(static_cast<uint64_t>(after));
        Difference(diff);
    }

    void OnSystemClockSynchronized(
        TTick before, TTick after, int64_t diff,
        const ESPressio::Timing::ClockSynchronizationResult<TTick>&,
        const ESPressio::Timing::ClockSynchronizationStatus<TTick>&
    ) override {
        Prefix("Synchronized");
        _output->print(" beforeNs="); UnsignedValue(static_cast<uint64_t>(before));
        _output->print(" afterNs="); UnsignedValue(static_cast<uint64_t>(after));
        Difference(diff);
    }

    void OnSystemClockSynchronizationSampleRejected(
        const ESPressio::Timing::ClockSynchronizationResult<TTick>&,
        const ESPressio::Timing::ClockSynchronizationStatus<TTick>&
    ) override { Prefix("SynchronizationSampleRejected"); _output->println(); }

    void OnSystemClockSynchronizationStateChanged(
        ESPressio::Timing::ClockSynchronizationState previous,
        ESPressio::Timing::ClockSynchronizationState current,
        const ESPressio::Timing::ClockSynchronizationStatus<TTick>&
    ) override {
        Prefix("SynchronizationStateChanged");
        _output->print(" previous="); _output->print((int)previous);
        _output->print(" current="); _output->println((int)current);
    }

    void OnSystemClockSynchronizationReset(
        const ESPressio::Timing::ClockSynchronizationStatus<TTick>&,
        const ESPressio::Timing::ClockSynchronizationStatus<TTick>&
    ) override { Prefix("SynchronizationReset"); _output->println(); }

    void OnSystemClockSynchronizationConfigurationChanged(
        const ESPressio::Timing::ClockSynchronizationConfig&,
        const ESPressio::Timing::ClockSynchronizationConfig&
    ) override { Prefix("SynchronizationConfigurationChanged"); _output->println(); }

    void OnSystemClockCallbackScheduled(TTick when) override {
        Prefix("CallbackScheduled"); _output->print(" scheduledNs="); _output->println((unsigned long long)when);
    }
    void OnSystemClockCallbackScheduleFailed(TTick when) override {
        Prefix("CallbackScheduleFailed"); _output->print(" scheduledNs="); _output->println((unsigned long long)when);
    }
    void OnSystemClockCallbackExecuted(TTick scheduled, TTick actual, int64_t diff) override {
        Prefix("CallbackExecuted");
        _output->print(" scheduledNs="); UnsignedValue(static_cast<uint64_t>(scheduled));
        _output->print(" actualNs="); UnsignedValue(static_cast<uint64_t>(actual));
        Difference(diff);
    }
    void OnSystemClockCallbackExecutionFailed(TTick scheduled, TTick actual, int64_t diff, std::exception_ptr) override {
        Prefix("CallbackExecutionFailed");
        _output->print(" scheduledNs="); UnsignedValue(static_cast<uint64_t>(scheduled));
        _output->print(" actualNs="); UnsignedValue(static_cast<uint64_t>(actual));
        Difference(diff);
    }
    void OnSystemClockCallbacksCleared(std::size_t count) override {
        Prefix("CallbacksCleared"); _output->print(" count="); _output->println((unsigned long)count);
    }
};

}
