#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <ESPressio_BinaryArchive.hpp>

#include "../ESPressio_SerialTypes.hpp"

namespace ESPressio::Serial {

inline Serializable::BinaryArchiveDecodeLimits
BuildEventMonitorDecodeLimits(
    const EventMonitorConfig& config
) noexcept {
    Serializable::BinaryArchiveDecodeLimits limits;

    limits.MaximumDepth =
        config.MaximumStructuredDepth;

    limits.MaximumTotalNodes =
        std::max<std::size_t>(
            config.MaximumStructuredNodes,
            1
        );

    const auto maximumCollectionItems =
        std::max<std::size_t>(
            config.MaximumCollectionItems,
            1
        );

    limits.MaximumObjectMembers =
        static_cast<uint32_t>(
            std::min<std::size_t>(
                maximumCollectionItems,
                UINT32_MAX
            )
        );

    limits.MaximumArrayElements =
        static_cast<uint32_t>(
            std::min<std::size_t>(
                maximumCollectionItems,
                UINT32_MAX
            )
        );

    limits.MaximumNameLength =
        std::max<std::size_t>(
            config.MaximumStringLength,
            1
        );

    limits.MaximumStringLength =
        std::max<std::size_t>(
            config.MaximumStringLength,
            1
        );

    return limits;
}


inline bool ValidateStructuredEventPayload(
    const uint8_t* payload,
    std::size_t size,
    const EventMonitorConfig& config
) noexcept {
    if (
        payload == nullptr ||
        size == 0
    ) {
        return false;
    }

    Serializable::BinaryArchive archive;

    return archive.Load(
        payload,
        size,
        BuildEventMonitorDecodeLimits(config)
    );
}

} // namespace ESPressio::Serial
