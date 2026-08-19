#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "ESPressio_SerializationResult.hpp"

namespace ESPressio::Serializable {

struct SerializationNode {};

struct PropertySchemaInfo {
    std::string Name;
    bool Required = false;
    bool ReadOnly = false;
    bool Sensitive = false;
    bool HasDefault = false;
    std::vector<std::string> Aliases;
    std::string Type;
};

}

namespace ESPressio::Event {

enum EventPriority {
    Low,
    Normal,
    High
};

enum EventDispatchMethod {
    Stack,
    Queue
};

enum class EventTransportDirection : uint8_t {
    None = 0,
    Inbound = 1,
    Outbound = 2,
    Bidirectional = 3
};

class IEvent {
public:
    virtual ~IEvent() = default;
};

class DummyRuntimeEvent final :
    public IEvent {};

template<typename TEvent>
struct EventTransportTypeTraits {
    static constexpr std::string_view
        Name{};
};

struct SerializableEventDescriptor {
    uint64_t TypeID = 0;
    std::string TypeName;
    uint32_t SchemaVersion = 1;
    EventTransportDirection DefaultDirection =
        EventTransportDirection::None;
    std::vector<
        Serializable::PropertySchemaInfo
    > Properties;
    bool CanConstruct = false;
};

struct SerializableEventConstructionResult {
    std::unique_ptr<IEvent> Event;
    Serializable::DeserializationResult
        Deserialization;
    bool TypeRegistered = false;
    bool Constructible = false;

    bool Success() const {
        return
            TypeRegistered &&
            Constructible &&
            Event != nullptr &&
            Deserialization.Success();
    }

    explicit operator bool() const {
        return Success();
    }
};

enum class RuntimeEventDispatchResult : uint8_t {
    Dispatched,
    NullEvent,
    UnsupportedMethod
};

class EventTransportManager {
public:
    static inline unsigned
        DispatchCount = 0;

    static EventTransportManager&
    GetInstance() {
        static EventTransportManager
            instance;

        return instance;
    }

    std::vector<
        SerializableEventDescriptor
    >
    GetRegisteredSerializableEvents()
        const {
        SerializableEventDescriptor
            descriptor;

        descriptor.TypeID = 1;
        descriptor.TypeName =
            "test.event.v1";
        descriptor.SchemaVersion = 1;
        descriptor.DefaultDirection =
            EventTransportDirection::
                Outbound;
        descriptor.CanConstruct = true;

        descriptor.Properties.push_back(
            {
                "value",
                true,
                false,
                false,
                false,
                {},
                "integer"
            }
        );

        return {descriptor};
    }

    bool FindRegisteredSerializableEvent(
        std::string_view typeName,
        SerializableEventDescriptor&
            descriptor
    ) const {
        if (
            typeName !=
            "test.event.v1"
        ) {
            return false;
        }

        descriptor =
            GetRegisteredSerializableEvents()
                .front();

        return true;
    }

    SerializableEventConstructionResult
    CreateSerializableEvent(
        uint64_t typeID,
        const Serializable::
            SerializationNode&,
        const Serializable::
            DeserializationOptions& = {}
    ) const {
        SerializableEventConstructionResult
            result;

        if (typeID != 1) {
            return result;
        }

        result.TypeRegistered = true;
        result.Constructible = true;
        result.Event =
            std::make_unique<
                DummyRuntimeEvent
            >();

        return result;
    }

    static RuntimeEventDispatchResult
    DispatchSerializableEvent(
        std::unique_ptr<IEvent> event,
        EventDispatchMethod = Queue,
        EventPriority = Normal
    ) {
        if (!event) {
            return
                RuntimeEventDispatchResult::
                    NullEvent;
        }

        ++DispatchCount;

        return
            RuntimeEventDispatchResult::
                Dispatched;
    }
};

}
