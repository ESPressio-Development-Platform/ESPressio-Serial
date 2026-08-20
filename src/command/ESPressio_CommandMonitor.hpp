#pragma once

#if !__has_include(<ESPressio_Command.hpp>)
#error "CommandMonitor requires ESPressio Command >= 0.3.0 < 1.0.0."
#endif

#include <Arduino.h>
#include <ESPressio_Command.hpp>
#include <ESPressio_ICommandRegistryObserver.hpp>

namespace ESPressio::Serial {

class CommandMonitor final :
    public ESPressio::Command::ICommandRegistryObserver {
private:
    Print* _output = nullptr;
    ESPressio::Observable::ObserverHandlePtr _handle;

    void Line(const char* operation, const std::vector<std::string>& path) {
        if (!_output) return;
        _output->print("[ESPressio Command] ");
        _output->print(operation);
        if (!path.empty()) {
            _output->print(" ");
            for (std::size_t i = 0; i < path.size(); ++i) {
                if (i) _output->print("/");
                _output->print(path[i].c_str());
            }
        }
        _output->println();
    }

public:
    bool Initialize(Print& output, ESPressio::Command::CommandRegistry& registry = ESPressio::Command::CommandRegistry::GetInstance()) {
        if (_handle) return true;
        _output = &output;
        _handle = registry.RegisterObserver(this);
        if (!_handle) { _output = nullptr; return false; }
        return true;
    }

    void Shutdown() {
        _handle.reset();
        _output = nullptr;
    }

    void OnCommandRegistered(const std::vector<std::string>& path) override {
        Line("Registered", path);
    }

    void OnCommandUnregistered(const std::vector<std::string>& path) override {
        Line("Unregistered", path);
    }
};

} // namespace ESPressio::Serial
