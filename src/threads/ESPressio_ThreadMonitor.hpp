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
    bool Initialize(Print& output) {
        if (_managerHandle || _terminationHandle) return true;
        _output = &output;
        _managerHandle = ESPressio::Threads::ThreadManager::GetInstance()->RegisterObserver(this);
        _terminationHandle = ESPressio::Threads::ThreadTerminationDispatcher::GetInstance()->RegisterObserver(this);
        if (!_managerHandle || !_terminationHandle) { Shutdown(); return false; }
        return true;
    }
    void Shutdown() {
        _terminationHandle.reset(); _managerHandle.reset(); _output=nullptr;
    }

    void OnThreadRegistered(ESPressio::Threads::IThread*, const ESPressio::Threads::ThreadManagerThreadSnapshot&) override { Line("Manager","ThreadRegistered"); }
    void OnThreadRegistrationFailed(ESPressio::Threads::IThread*, std::exception_ptr) override { Line("Manager","ThreadRegistrationFailed"); }
    void OnThreadRemoved(const ESPressio::Threads::ThreadManagerThreadSnapshot&) override { Line("Manager","ThreadRemoved"); }
    void OnThreadCleanupClaimed(ESPressio::Threads::IThread*, const ESPressio::Threads::ThreadManagerThreadSnapshot&) override { Line("Manager","ThreadCleanupClaimed"); }
    void OnThreadCleanupDeferred(const ESPressio::Threads::ThreadManagerCleanupResult&) override { Line("Manager","ThreadCleanupDeferred"); }
    void OnThreadCleanupStarted(const ESPressio::Threads::ThreadManagerCleanupResult&) override { Line("Manager","ThreadCleanupStarted"); }
    void OnThreadCleanupCompleted(const ESPressio::Threads::ThreadManagerCleanupResult&) override { Line("Manager","ThreadCleanupCompleted"); }
    void OnThreadCleanupFailed(const ESPressio::Threads::ThreadManagerCleanupResult&, std::exception_ptr) override { Line("Manager","ThreadCleanupFailed"); }
    void OnThreadManagerInitializationCompleted(const ESPressio::Threads::ThreadManagerInitializationResult&) override { Line("Manager","InitializationCompleted"); }

    void OnThreadTerminationDispatcherInitialized(bool) override { Line("Termination","Initialized"); }
    void OnThreadTerminationDispatchQueued(const ESPressio::Threads::ThreadManagerThreadSnapshot&) override { Line("Termination","DispatchQueued"); }
    void OnThreadTerminationDispatchQueueFailed(const ESPressio::Threads::ThreadManagerThreadSnapshot&) override { Line("Termination","DispatchQueueFailed"); }
    void OnThreadTerminationDispatchStarted(const ESPressio::Threads::ThreadManagerThreadSnapshot&) override { Line("Termination","DispatchStarted"); }
    void OnThreadTerminationDispatchCompleted(const ESPressio::Threads::ThreadManagerThreadSnapshot&) override { Line("Termination","DispatchCompleted"); }
};

}
