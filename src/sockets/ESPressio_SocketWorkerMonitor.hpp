#pragma once

#if !__has_include(<ESPressio_SocketWorker.hpp>)
#error "SocketWorkerMonitor requires ESPressio Sockets >= 0.5.0 < 1.0.0."
#endif

#include <Arduino.h>
#include <ESPressio_ISocketWorkerObserver.hpp>
#include <ESPressio_SocketWorker.hpp>

namespace ESPressio::Serial {

/// <summary>Writes SocketWorker start/stop lifecycle activity to an Arduino Print sink.</summary>
class SocketWorkerMonitor final :
    public ESPressio::Sockets::ISocketWorkerObserver {
private:
    Print* _output = nullptr;
    ESPressio::Observable::ObserverHandlePtr _handle;

    void Line(const char* operation, const char* name = nullptr) {
        if (!_output) return;
        _output->print("[ESPressio Sockets] [Worker] ");
        _output->print(operation);
        if (name != nullptr) {
            _output->print(" name=");
            _output->print(name);
        }
        _output->println();
    }

public:
    /// <summary>Registers the monitor with a SocketWorker and selects its output sink.</summary>
    /// <returns>True when the observer registration is active.</returns>
    bool Initialize(Print& output, ESPressio::Sockets::SocketWorker& worker) {
        if (_handle) return true;
        _output = &output;
        _handle = worker.RegisterObserver(this);
        if (!_handle) { _output = nullptr; return false; }
        return true;
    }

    /// <summary>Unregisters from the worker and releases the output sink reference.</summary>
    void Shutdown() {
        _handle.reset();
        _output = nullptr;
    }

    /// <inheritdoc/>
    void OnSocketWorkerStarted(const char* name) override { Line("Started", name); }
    /// <inheritdoc/>
    void OnSocketWorkerStartFailed(const char* name) override { Line("StartFailed", name); }
    /// <inheritdoc/>
    void OnSocketWorkerStopped() override { Line("Stopped"); }
};

} // namespace ESPressio::Serial
