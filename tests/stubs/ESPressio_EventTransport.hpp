#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "ESPressio_SerializationResult.hpp"

namespace ESPressio::Serializable {

/**
 * ESPressio Memory Audit
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 0 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
struct SerializationNode {};

/**
 * ESPressio Memory Audit
 * Members:
 * - Name (std::string): 24 bytes [Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * - Required (bool): 1 bytes [0 bytes dynamic allocation]
 * - ReadOnly (bool): 1 bytes [0 bytes dynamic allocation]
 * - Sensitive (bool): 1 bytes [0 bytes dynamic allocation]
 * - HasDefault (bool): 1 bytes [0 bytes dynamic allocation]
 * - Aliases (std::vector<std::string>): 12 bytes [Capacity * 24 bytes; N elements each may add: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * - Type (std::string): 24 bytes [Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Total Memory: 64 bytes [Name: Capacity + 1 bytes when capacity exceeds 15-byte SSO; Aliases: Capacity * 24 bytes; Aliases: N elements each may add: Capacity + 1 bytes when capacity exceeds 15-byte SSO; Type: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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

/**
 * ESPressio Memory Audit
 * Underlying storage: 4 bytes
 * Total Memory: 4 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
enum EventPriority {
    Low,
    Normal,
    High
};

/**
 * ESPressio Memory Audit
 * Underlying storage: 4 bytes
 * Total Memory: 4 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
enum EventDispatchMethod {
    Stack,
    Queue
};

/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
enum
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
class EventTransportDirection : uint8_t {
    None = 0,
    Inbound = 1,
    Outbound = 2,
    Bidirectional = 3
};

/**
 * ESPressio Memory Audit
 * Members: none; polymorphic interface/object includes vptr storage where not supplied by a base.
 * Total Memory: 4 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
class IEvent {
public:
    virtual ~IEvent() = default;
};

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 4 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
class DummyRuntimeEvent final :
    public IEvent {};

/**
 * ESPressio Memory Audit
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 0 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
template<typename TEvent>
struct EventTransportTypeTraits {
    static constexpr std::string_view
        Name{};
};

/**
 * ESPressio Memory Audit
 * Members:
 * - TypeID (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - TypeName (std::string): 24 bytes [Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * - SchemaVersion (uint32_t): 4 bytes [0 bytes dynamic allocation]
 * - DefaultDirection (EventTransportDirection): 1 bytes [0 bytes dynamic allocation]
 * - Properties (std::vector<Serializable::PropertySchemaInfo>): 12 bytes [Capacity * 64 bytes; N elements each may add: Name: Capacity + 1 bytes when capacity exceeds 15-byte SSO + Aliases: Capacity * 24 bytes + Aliases: N elements each may add: Capacity + 1 bytes when capacity exceeds 15-byte SSO + Type: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * - CanConstruct (bool): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: 56 bytes [TypeName: Capacity + 1 bytes when capacity exceeds 15-byte SSO; Properties: Capacity * 64 bytes; Properties: N elements each may add: Name: Capacity + 1 bytes when capacity exceeds 15-byte SSO + Aliases: Capacity * 24 bytes + Aliases: N elements each may add: Capacity + 1 bytes when capacity exceeds 15-byte SSO + Type: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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

/**
 * ESPressio Memory Audit
 * Members:
 * - Event (std::unique_ptr<IEvent>): 4 bytes [owned object: 4 bytes]
 * - Deserialization (Serializable::DeserializationResult): 12 bytes [_issues: Capacity * 52 bytes; _issues: N elements each may add: Path: Capacity + 1 bytes when capacity exceeds 15-byte SSO + Message: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * - TypeRegistered (bool): 1 bytes [0 bytes dynamic allocation]
 * - Constructible (bool): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: 20 bytes [Event: owned object: 4 bytes; Deserialization: _issues: Capacity * 52 bytes; Deserialization: _issues: N elements each may add: Path: Capacity + 1 bytes when capacity exceeds 15-byte SSO + Message: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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

/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
enum
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
class RuntimeEventDispatchResult : uint8_t {
    Dispatched,
    NullEvent,
    UnsupportedMethod
};

/**
 * ESPressio Memory Audit
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 0 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
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
