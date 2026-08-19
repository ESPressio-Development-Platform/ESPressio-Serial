#pragma once

#include <string>

#include "ESPressio_EventTransport.hpp"

namespace ESPressio::Serializable {

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
