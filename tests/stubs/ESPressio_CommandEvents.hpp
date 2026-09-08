#pragma once

#include <functional>

#include <ESPressio_CommandEnvelope.hpp>

namespace ESPressio {
namespace Event {

/**
 * ESPressio Memory Audit
 * Members:
 * - Envelope (Command::CommandRequestEnvelope): sizeof(Command::CommandRequestEnvelope) [0 bytes dynamic allocation]
 * Total Memory: sizeof(Command::CommandRequestEnvelope) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
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
