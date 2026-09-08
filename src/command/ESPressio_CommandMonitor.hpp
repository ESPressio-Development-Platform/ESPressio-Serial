#pragma once

#if !__has_include(<ESPressio_Command.hpp>)
#error "CommandMonitor requires ESPressio Command >= 0.3.0 < 1.0.0."
#endif

#include <Arduino.h>
#include <ESPressio_Command.hpp>
#include <ESPressio_ICommandRegistryObserver.hpp>

namespace ESPressio::Serial {

/// <summary>Writes Command registry registration activity to an Arduino Print sink.</summary>
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members:
 * - _output (Print*): 4 bytes [0 bytes dynamic allocation]
 * - _handle (ESPressio::Observable::ObserverHandlePtr): 12 bytes [owned object: 4 bytes]
 * Total Memory: 20 bytes [_handle: owned object: 4 bytes]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class CommandMonitor final :
    public ESPressio::Command::ICommandRegistryObserver {
private:
    Print* _output = nullptr;
    ESPressio::Observable::ObserverHandlePtr _handle;

    /// <summary>Writes one borrowed allocator-aware Command path without materializing a diagnostic copy.</summary>
    void Line(
        const char* operation,
        const ESPressio::Command::CommandPath& path
    ) {
        if (!_output) return;
        _output->print("[ESPressio Command] ");
        _output->print(operation);
        if (!path.empty()) {
            _output->print(" ");
            for (std::size_t i = 0; i < path.size(); ++i) {
                if (i) _output->print("/");
                _output->write(
                    reinterpret_cast<const uint8_t*>(path[i].data()),
                    path[i].size()
                );
            }
        }
        _output->println();
    }

public:
    /// <summary>Registers the monitor with a Command registry and selects its output sink.</summary>
    /// <param name="output">Destination for diagnostic lines.</param>
    /// <param name="registry">Registry to observe; defaults to the process-wide Command registry.</param>
    /// <returns>True when observation is active.</returns>
    bool Initialize(Print& output, ESPressio::Command::CommandRegistry& registry = ESPressio::Command::CommandRegistry::GetInstance()) {
        if (_handle) return true;
        _output = &output;
        _handle = registry.RegisterObserver(this);
        if (!_handle) { _output = nullptr; return false; }
        return true;
    }

    /// <summary>Unregisters the monitor and releases the output sink reference.</summary>
    void Shutdown() {
        _handle.reset();
        _output = nullptr;
    }

    /// <summary>Writes a diagnostic line when a Command path is registered.</summary>
    void OnCommandRegistered(
        const ESPressio::Command::CommandPath& path
    ) override {
        Line("Registered", path);
    }

    /// <summary>Writes a diagnostic line when a Command path is unregistered.</summary>
    void OnCommandUnregistered(
        const ESPressio::Command::CommandPath& path
    ) override {
        Line("Unregistered", path);
    }
};

} // namespace ESPressio::Serial
