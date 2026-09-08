#pragma once

#include <cstddef>
#include <cstdint>

namespace ESPressio::Serial {

/// <summary>Selects whether Event monitoring reports event traffic or event lifecycle activity.</summary>
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
class EventMonitorMode : uint8_t {
    /// <summary>Report dispatched/transported events.</summary>
    Events,
    /// <summary>Report event lifecycle activity.</summary>
    Lifecycle
};

/// <summary>Selects how an Event monitor renders event payload content.</summary>
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
class EventMonitorPayloadFormat : uint8_t {
    /// <summary>Do not render payload content.</summary>
    None,
    /// <summary>Render a compact payload summary.</summary>
    Summary,
    /// <summary>Render payload bytes as hexadecimal.</summary>
    Hex,
    /// <summary>Render payloads using their structured serialization representation.</summary>
    Structured
};

/// <summary>Controls Event monitor direction, metadata, payload formatting, and output bounds.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - Mode (EventMonitorMode): 1 bytes [0 bytes dynamic allocation]
 * - PayloadFormat (EventMonitorPayloadFormat): 1 bytes [0 bytes dynamic allocation]
 * - ShowInbound (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowOutbound (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowFailures (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowStableTypeName (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowTypeID (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowSchemaVersion (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowMessageID (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowTransportAddress (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowDispatchMetadata (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowOrigin (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowHopCount (bool): 1 bytes [0 bytes dynamic allocation]
 * - ShowTransportAccepted (bool): 1 bytes [0 bytes dynamic allocation]
 * - PrettyStructuredPayload (bool): 1 bytes [0 bytes dynamic allocation]
 * - MaximumRenderedBytes (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * - MaximumHexPayloadBytes (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * - MaximumCollectionItems (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * - MaximumStringLength (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * - MaximumStructuredNodes (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * - MaximumStructuredDepth (uint8_t): 1 bytes [0 bytes dynamic allocation]
 * - IndentSpaces (uint8_t): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: 40 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
struct EventMonitorConfig {
    /// <summary>Activity category reported by the monitor.</summary>
    EventMonitorMode Mode =
        EventMonitorMode::Events;

    /// <summary>Payload representation used when event payloads are displayed.</summary>
    EventMonitorPayloadFormat PayloadFormat =
        EventMonitorPayloadFormat::Structured;

    /// <summary>Whether inbound event activity is displayed.</summary>
    bool ShowInbound = true;
    /// <summary>Whether outbound event activity is displayed.</summary>
    bool ShowOutbound = true;
    /// <summary>Whether failures are displayed.</summary>
    bool ShowFailures = true;

    /// <summary>Whether stable event type names are displayed.</summary>
    bool ShowStableTypeName = true;
    /// <summary>Whether event type identifiers are displayed.</summary>
    bool ShowTypeID = true;
    /// <summary>Whether serialization schema versions are displayed.</summary>
    bool ShowSchemaVersion = true;
    /// <summary>Whether message identifiers are displayed.</summary>
    bool ShowMessageID = true;
    /// <summary>Whether transport addressing is displayed.</summary>
    bool ShowTransportAddress = true;
    /// <summary>Whether local dispatch metadata is displayed.</summary>
    bool ShowDispatchMetadata = true;
    /// <summary>Whether event-origin metadata is displayed.</summary>
    bool ShowOrigin = true;
    /// <summary>Whether transport hop counts are displayed.</summary>
    bool ShowHopCount = true;
    /// <summary>Whether transport-acceptance state is displayed.</summary>
    bool ShowTransportAccepted = true;

    /// <summary>Whether structured payloads are pretty-printed.</summary>
    bool PrettyStructuredPayload = true;

    /// <summary>Maximum complete diagnostic record bytes composed before one serialized write to the output sink.</summary>
    /// <remarks>The buffer is backed by the ESPressio System ExternalPreferred memory policy so verbose diagnostics do not consume scarce internal heap. Output beyond this bound is truncated.</remarks>
    std::size_t MaximumRenderedBytes = 4096;
    /// <summary>Maximum payload bytes rendered in hexadecimal form.</summary>
    std::size_t MaximumHexPayloadBytes = 256;
    /// <summary>Maximum collection elements rendered from a structured payload.</summary>
    std::size_t MaximumCollectionItems = 64;
    /// <summary>Maximum string length rendered from a structured payload.</summary>
    std::size_t MaximumStringLength = 512;
    /// <summary>Maximum structured nodes visited while rendering a payload.</summary>
    std::size_t MaximumStructuredNodes = 1024;
    /// <summary>Maximum structured nesting depth rendered from a payload.</summary>
    uint8_t MaximumStructuredDepth = 12;
    /// <summary>Spaces used for each indentation level in pretty structured output.</summary>
    uint8_t IndentSpaces = 2;
};

}
