#pragma once

#include <cstddef>
#include <cstdint>

#include <ESPressio_EventTransport.hpp>
#include <ESPressio_SerializationResult.hpp>

namespace ESPressio::Serial {

/// <summary>Controls which registered serializable Event types may be constructed and dispatched interactively.</summary>
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
class EventConsoleAccessPolicy : uint8_t {
    AllowListedOnly,
    AllRegistered
};

/// <summary>Configures EventConsole access control, dispatch confirmation, schema visibility, and JSON deserialization limits.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - AccessPolicy (EventConsoleAccessPolicy): 1 bytes [0 bytes dynamic allocation]
 * - RequireConfirmation (bool): 1 bytes [0 bytes dynamic allocation]
 * - AllowQueue (bool): 1 bytes [0 bytes dynamic allocation]
 * - AllowStack (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowDeniedEventsInList (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowAliases (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowSensitivePropertyMetadata (bool): 1 bytes [0 bytes dynamic allocation]
 * - DefaultPriority (Event::EventPriority): 4 bytes [0 bytes dynamic allocation]
 * - MaximumJsonLength (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 16 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
struct EventConsoleConfig {
    /// <summary>Policy used to decide whether a registered Event type is operator-accessible.</summary>
    EventConsoleAccessPolicy AccessPolicy =
        EventConsoleAccessPolicy::
            AllowListedOnly;

    /// <summary>Require explicit confirmation before dispatching a constructed Event.</summary>
    bool RequireConfirmation = true;
    /// <summary>Permit queue dispatch from the console.</summary>
    bool AllowQueue = true;
    /// <summary>Permit stack dispatch from the console.</summary>
    bool AllowStack = true;

    /// <summary>Include access-denied Event types in interactive listings.</summary>
    bool ShowDeniedEventsInList = true;
    /// <summary>Include Serializable property aliases when describing Event schemas.</summary>
    bool ShowAliases = true;
    /// <summary>Expose whether properties are marked sensitive when describing schemas; values are not exposed by this flag.</summary>
    bool ShowSensitivePropertyMetadata = true;

    /// <summary>Default priority used when the operator does not specify one.</summary>
    Event::EventPriority DefaultPriority =
        Event::EventPriority::Normal;

    /// <summary>Options passed to Serializable when constructing Event instances from operator-supplied JSON.</summary>
    Serializable::DeserializationOptions
        DeserializationOptions{};

    /// <summary>Maximum accepted JSON input length for interactive Event construction.</summary>
    std::size_t MaximumJsonLength = 1800;
};

}
