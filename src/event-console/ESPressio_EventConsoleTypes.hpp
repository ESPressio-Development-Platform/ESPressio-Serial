#pragma once

#include <cstddef>
#include <cstdint>

#include <ESPressio_EventTransport.hpp>
#include <ESPressio_SerializationResult.hpp>

namespace ESPressio::Serial {

enum class EventConsoleAccessPolicy : uint8_t {
    AllowListedOnly,
    AllRegistered
};

struct EventConsoleConfig {
    EventConsoleAccessPolicy AccessPolicy =
        EventConsoleAccessPolicy::
            AllowListedOnly;

    bool RequireConfirmation = true;
    bool AllowQueue = true;
    bool AllowStack = true;

    bool ShowDeniedEventsInList = true;
    bool ShowAliases = true;
    bool ShowSensitivePropertyMetadata = true;

    Event::EventPriority DefaultPriority =
        Event::EventPriority::Normal;

    Serializable::DeserializationOptions
        DeserializationOptions{};

    std::size_t MaximumJsonLength = 1800;
};

}
