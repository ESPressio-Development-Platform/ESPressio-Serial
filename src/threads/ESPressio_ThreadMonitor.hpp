#pragma once
#if !__has_include(<ESPressio_ThreadManager.hpp>)
#error "ThreadMonitor requires ESPressio Threads >= 3.1.1 < 4.0.0."
#endif
#include <Arduino.h>
#include <ESPressio_IThreadManagerObserver.hpp>
#include <ESPressio_IThreadTerminationDispatcherObserver.hpp>
#include <ESPressio_ThreadManager.hpp>
#include <ESPressio_ThreadTerminationDispatcher.hpp>

namespace ESPressio::Serial {

/// <summary>Writes ThreadManager and termination-dispatcher lifecycle activity to an Arduino Print sink.</summary>
class ThreadMonitor final :
    public ESPressio::Threads::IThreadManagerObserver,
    public ESPressio::Threads::IThreadTerminationDispatcherObserver {
    Print* _output = nullptr;
    ESPressio::Observable::ObserverHandlePtr _managerHandle, _terminationHandle;
    void Line(const char* subsystem, const char* operation) {
        if (!_output) return;
        _output->print("[ESPressio Threads] ["); _output->print(subsystem);
        _output->print("] "); _output->println(operation);
    }
public:
    /// <summary>Registers the monitor with both the process-wide ThreadManager and termination dispatcher.</summary>
    /// <param name="output">Destination for diagnostic lines.</param>
    /// <returns>True when both observer registrations are active.</returns>
    bool Initialize(Print& output) {
        if (_managerHandle || _terminationHandle) return true;
        _output = &output;
        _managerHandle = ESPressio::Threads::ThreadManager::GetInstance()->RegisterObserver(this);
        _terminationHandle = ESPressio::Threads::ThreadTerminationDispatcher::GetInstance()->RegisterObserver(this);
        if (!_managerHandle || !_terminationHandle) { Shutdown(); return false; }
        return true;
    }

    /// <summary>Unregisters both observer handles and releases the output sink reference.</summary>
    void Shutdown() {
        _terminationHandle.reset(); _managerHandle.reset(); _output=nullptr;
    }

    /// <inheritdoc/>
    void OnThreadRegistered(ESPressio::Threads::IThread*, const ESPressio::Threads::ThreadManagerThreadSnapshot&) override { Line("Manager","ThreadRegistered"); }
    /// <inheritdoc/>
    void OnThreadRegistrationFailed(ESPressio::Threads::IThread*, std::exception_ptr) override { Line("Manager","ThreadRegistrationFailed"); }
    /// <inheritdoc/>
    void OnThreadRemoved(const ESPressio::Threads::ThreadManagerThreadSnapshot&) override { Line("Manager","ThreadRemoved"); }
    /// <inheritdoc/>
    void OnThreadCleanupClaimed(ESPressio::Threads::IThread*, const ESPressio::Threads::ThreadManagerThreadSnapshot&) override { Line("Manager","ThreadCleanupClaimed"); }
    /// <inheritdoc/>
    void OnThreadCleanupDeferred(const ESPressio::Threads::ThreadManagerCleanupResult&) override { Line("Manager","ThreadCleanupDeferred"); }
    /// <inheritdoc/>
    void OnThreadCleanupStarted(const ESPressio::Threads::ThreadManagerCleanupResult&) override { Line("Manager","ThreadCleanupStarted"); }
    /// <inheritdoc/>
    void OnThreadCleanupCompleted(const ESPressio::Threads::ThreadManagerCleanupResult&) override { Line("Manager","ThreadCleanupCompleted"); }
    /// <inheritdoc/>
    void OnThreadCleanupFailed(const ESPressio::Threads::ThreadManagerCleanupResult&, std::exception_ptr) override { Line("Manager","ThreadCleanupFailed"); }
    /// <inheritdoc/>
    void OnThreadManagerInitializationCompleted(const ESPressio::Threads::ThreadManagerInitializationResult&) override { Line("Manager","InitializationCompleted"); }

    /// <inheritdoc/>
    void OnThreadTerminationDispatcherInitialized(bool) override { Line("Termination","Initialized"); }
    /// <inheritdoc/>
    void OnThreadTerminationDispatchQueued(const ESPressio::Threads::ThreadManagerThreadSnapshot&) override { Line("Termination","DispatchQueued"); }
    /// <inheritdoc/>
    void OnThreadTerminationDispatchQueueFailed(const ESPressio::Threads::ThreadManagerThreadSnapshot&) override { Line("Termination","DispatchQueueFailed"); }
    /// <inheritdoc/>
    void OnThreadTerminationDispatchStarted(const ESPressio::Threads::ThreadManagerThreadSnapshot&) override { Line("Termination","DispatchStarted"); }
    /// <inheritdoc/>
    void OnThreadTerminationDispatchCompleted(const ESPressio::Threads::ThreadManagerThreadSnapshot&) override { Line("Termination","DispatchCompleted"); }
};

}
