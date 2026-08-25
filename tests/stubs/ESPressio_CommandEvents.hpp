#pragma once

#include <functional>

#include <ESPressio_CommandEnvelope.hpp>

namespace ESPressio {
namespace Event {

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
