#pragma once

#if !__has_include(<ESPressio_IThread.hpp>)
#error "ThreadMonitor requires the final ESPressio Threads IThread diagnostics contract."
#endif

#include <Arduino.h>
#include <cinttypes>
#include <cstdio>
#include <ESPressio_IThread.hpp>

namespace ESPressio::Serial {

/// <summary>Prints a bounded point-in-time diagnostic snapshot for a caller-owned Thread.</summary>
/// <remarks>
/// The monitor owns no Thread registry, lifecycle dispatcher, worker, or scheduling topology. The application supplies
/// the exact IThread instance it owns and Serial reads only the final per-instance diagnostics surface.
/// </remarks>
class ThreadMonitor final {
private:
    Print* _output = nullptr;

    void PrintUnsigned(std::uint64_t value) {
        char buffer[32]{};
        std::snprintf(buffer, sizeof(buffer), "%" PRIu64, value);
        _output->print(buffer);
    }

    static const char* StateName(Threads::ThreadState state) noexcept {
        switch (state) {
            case Threads::ThreadState::Uninitialized: return "uninitialized";
            case Threads::ThreadState::Initializing: return "initializing";
            case Threads::ThreadState::Initialized: return "initialized";
            case Threads::ThreadState::Running: return "running";
            case Threads::ThreadState::Paused: return "paused";
            case Threads::ThreadState::Terminating: return "terminating";
            case Threads::ThreadState::Terminated: return "terminated";
        }
        return "unknown";
    }

    static const char* FailurePhaseName(Threads::ThreadFailurePhase phase) noexcept {
        switch (phase) {
            case Threads::ThreadFailurePhase::None: return "none";
            case Threads::ThreadFailurePhase::Initialization: return "initialization";
            case Threads::ThreadFailurePhase::Lifecycle: return "lifecycle";
            case Threads::ThreadFailurePhase::Capability: return "capability";
            case Threads::ThreadFailurePhase::Application: return "application";
            case Threads::ThreadFailurePhase::Teardown: return "teardown";
            case Threads::ThreadFailurePhase::Provider: return "provider";
        }
        return "unknown";
    }

public:
    bool Initialize(Print& output) noexcept {
        _output = &output;
        return true;
    }

    void Shutdown() noexcept { _output = nullptr; }
    bool IsInitialized() const noexcept { return _output != nullptr; }

    /// <summary>Prints one immutable diagnostic snapshot without registering or mutating the Thread.</summary>
    bool PrintStatus(const Threads::IThread& thread, const char* name = nullptr) {
        if (_output == nullptr) return false;
        const auto diagnostics = thread.GetDiagnostics();

        _output->print("[ESPressio Threads]");
        if (name != nullptr && *name != '\0') {
            _output->print(" [");
            _output->print(name);
            _output->print("]");
        }
        _output->print(" state=");
        _output->print(StateName(diagnostics.State));
        _output->print(" service-cycles=");
        PrintUnsigned(diagnostics.ServiceCycles);
        _output->print(" application-iterations=");
        PrintUnsigned(diagnostics.ApplicationIterations);
        _output->print(" waits=");
        PrintUnsigned(diagnostics.Waits);
        _output->print(" failure-phase=");
        _output->print(FailurePhaseName(diagnostics.Failure.Phase));
        _output->print(" failure-time-ns=");
        PrintUnsigned(diagnostics.Failure.MonotonicNanoseconds);
        _output->print(" stack-telemetry=");
        _output->print(diagnostics.StackTelemetryAvailable ? "available" : "unavailable");
        if (diagnostics.StackTelemetryAvailable) {
            _output->print(" minimum-free-stack-bytes=");
            _output->print(static_cast<unsigned long>(diagnostics.MinimumFreeStackBytes));
        }
        _output->println();
        return true;
    }
};

} // namespace ESPressio::Serial
