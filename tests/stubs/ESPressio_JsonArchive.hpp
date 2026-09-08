#pragma once

#include <string>

#include "ESPressio_EventTransport.hpp"

namespace ESPressio::Serializable {

/**
 * ESPressio Memory Audit
 * Members:
 * - _node (SerializationNode): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class JsonArchive {
private:
    SerializationNode _node;

public:
    bool Load(
        const std::string& input
    ) {
        return
            !input.empty() &&
            input.front() == '{' &&
            input.back() == '}';
    }

    const SerializationNode&
    GetNode() const {
        return _node;
    }
};

}
