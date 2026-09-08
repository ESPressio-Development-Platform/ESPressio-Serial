#pragma once

#include <functional>

#include <ESPressio_CommandEnvelope.hpp>

namespace ESPressio {
namespace Event {

/**
 * ESPressio Memory Audit
 * Members:
 * - Envelope (Command::CommandRequestEnvelope): sizeof(Command::CommandRequestEnvelope) (target/toolchain dependent) [0 bytes dynamic allocation]
 * Total Memory: sizeof(Command::CommandRequestEnvelope) (target/toolchain dependent) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class InboundCommandEvent {
public:
    using QueueObserver = std::function<void(const Command::CommandRequestEnvelope&)>;

    inline static QueueObserver OnQueue{};

    Command::CommandRequestEnvelope Envelope;

    explicit InboundCommandEvent(const Command::CommandRequestEnvelope& envelope)
        : Envelope(envelope) {
    }

    void Queue() {
        if (OnQueue) {
            OnQueue(Envelope);
        }
        delete this;
    }
};

}
}
