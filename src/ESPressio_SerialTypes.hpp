#pragma once

#include <cstddef>
#include <cstdint>

namespace ESPressio::Serial {

enum class EventMonitorMode : uint8_t {
    Events,
    Lifecycle
};

enum class EventMonitorPayloadFormat : uint8_t {
    None,
    Summary,
    Hex,
    Structured
};

struct EventMonitorConfig {
    EventMonitorMode Mode =
        EventMonitorMode::Events;

    EventMonitorPayloadFormat PayloadFormat =
        EventMonitorPayloadFormat::Structured;

    bool ShowInbound = true;
    bool ShowOutbound = true;
    bool ShowFailures = true;

    bool ShowStableTypeName = true;
    bool ShowTypeID = true;
    bool ShowSchemaVersion = true;
    bool ShowMessageID = true;
    bool ShowTransportAddress = true;
    bool ShowDispatchMetadata = true;
    bool ShowOrigin = true;
    bool ShowHopCount = true;
    bool ShowTransportAccepted = true;

    bool PrettyStructuredPayload = true;

    std::size_t MaximumHexPayloadBytes = 256;
    std::size_t MaximumCollectionItems = 64;
    std::size_t MaximumStringLength = 512;
    uint8_t MaximumStructuredDepth = 12;
    uint8_t IndentSpaces = 2;
};

}
