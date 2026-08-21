#pragma once

#include <algorithm>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include <Arduino.h>
#include <ESPressio_BinaryArchiveTraversal.hpp>

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
    return Serializable::ValidateBinaryArchive(
        payload,
        size,
        BuildEventMonitorDecodeLimits(config)
    );
}


namespace EventMonitorPayloadSafetyDetail {

inline void PrintIndent(
    Print& output,
    std::size_t depth,
    uint8_t spaces
) noexcept {
    const std::size_t count =
        depth * static_cast<std::size_t>(spaces);

    for (std::size_t index = 0; index < count; ++index) {
        output.write(static_cast<uint8_t>(' '));
    }
}


inline void PrintEscapedString(
    Print& output,
    std::string_view value,
    std::size_t maximumLength
) noexcept {
    output.write(static_cast<uint8_t>('"'));

    const std::size_t length =
        std::min(value.size(), maximumLength);

    for (std::size_t index = 0; index < length; ++index) {
        const unsigned char character =
            static_cast<unsigned char>(value[index]);

        switch (character) {
            case '"': output.print("\\\""); break;
            case '\\': output.print("\\\\"); break;
            case '\b': output.print("\\b"); break;
            case '\f': output.print("\\f"); break;
            case '\n': output.print("\\n"); break;
            case '\r': output.print("\\r"); break;
            case '\t': output.print("\\t"); break;

            default:
                if (character < 0x20u) {
                    char escaped[7];
                    std::snprintf(
                        escaped,
                        sizeof(escaped),
                        "\\u%04X",
                        static_cast<unsigned int>(character)
                    );
                    output.print(escaped);
                } else {
                    output.write(static_cast<uint8_t>(character));
                }
                break;
        }
    }

    if (value.size() > maximumLength) {
        output.print("...");
    }

    output.write(static_cast<uint8_t>('"'));
}


class StructuredPayloadPrinter final :
    public Serializable::BinaryArchiveVisitor {
private:
    Print& _output;
    const EventMonitorConfig& _config;

    void BeforeCollectionItem(
        uint32_t index,
        std::size_t depth
    ) noexcept {
        if (index > 0) {
            _output.write(static_cast<uint8_t>(','));
        }

        if (_config.PrettyStructuredPayload) {
            _output.println();
            PrintIndent(
                _output,
                depth + 1,
                _config.IndentSpaces
            );
        }
    }

    void CloseCollection(
        uint32_t count,
        std::size_t depth,
        char close
    ) noexcept {
        if (
            _config.PrettyStructuredPayload &&
            count > 0
        ) {
            _output.println();
            PrintIndent(
                _output,
                depth,
                _config.IndentSpaces
            );
        }

        _output.write(static_cast<uint8_t>(close));
    }

public:
    StructuredPayloadPrinter(
        Print& output,
        const EventMonitorConfig& config
    ) noexcept :
        _output(output),
        _config(config) {
    }

    bool OnObjectBegin(
        uint32_t,
        std::size_t
    ) noexcept override {
        _output.write(static_cast<uint8_t>('{'));
        return true;
    }

    bool OnObjectProperty(
        std::string_view name,
        uint32_t index,
        uint32_t,
        std::size_t depth
    ) noexcept override {
        BeforeCollectionItem(index, depth);
        PrintEscapedString(
            _output,
            name,
            _config.MaximumStringLength
        );
        _output.print(
            _config.PrettyStructuredPayload
                ? ": "
                : ":"
        );
        return true;
    }

    bool OnObjectEnd(
        uint32_t count,
        std::size_t depth
    ) noexcept override {
        CloseCollection(count, depth, '}');
        return true;
    }

    bool OnArrayBegin(
        uint32_t,
        std::size_t
    ) noexcept override {
        _output.write(static_cast<uint8_t>('['));
        return true;
    }

    bool OnArrayElement(
        uint32_t index,
        uint32_t,
        std::size_t depth
    ) noexcept override {
        BeforeCollectionItem(index, depth);
        return true;
    }

    bool OnArrayEnd(
        uint32_t count,
        std::size_t depth
    ) noexcept override {
        CloseCollection(count, depth, ']');
        return true;
    }

    bool OnNull(
        std::size_t
    ) noexcept override {
        _output.print("null");
        return true;
    }

    bool OnBoolean(
        bool value,
        std::size_t
    ) noexcept override {
        _output.print(value ? "true" : "false");
        return true;
    }

    bool OnSignedInteger(
        int64_t value,
        std::size_t
    ) noexcept override {
        char buffer[32];
        std::snprintf(
            buffer,
            sizeof(buffer),
            "%" PRId64,
            value
        );
        _output.print(buffer);
        return true;
    }

    bool OnUnsignedInteger(
        uint64_t value,
        std::size_t
    ) noexcept override {
        char buffer[32];
        std::snprintf(
            buffer,
            sizeof(buffer),
            "%" PRIu64,
            value
        );
        _output.print(buffer);
        return true;
    }

    bool OnFloat32(
        float value,
        std::size_t
    ) noexcept override {
        char buffer[32];
        std::snprintf(
            buffer,
            sizeof(buffer),
            "%.7g",
            static_cast<double>(value)
        );
        _output.print(buffer);
        return true;
    }

    bool OnFloat64(
        double value,
        std::size_t
    ) noexcept override {
        char buffer[48];
        std::snprintf(
            buffer,
            sizeof(buffer),
            "%.15g",
            value
        );
        _output.print(buffer);
        return true;
    }

    bool OnString(
        std::string_view value,
        std::size_t
    ) noexcept override {
        PrintEscapedString(
            _output,
            value,
            _config.MaximumStringLength
        );
        return true;
    }
};


inline void PrintHexPayload(
    Print& output,
    const uint8_t* payload,
    std::size_t size,
    std::size_t maximumBytes
) noexcept {
    if (
        payload == nullptr ||
        size == 0
    ) {
        output.print("<none>");
        return;
    }

    const std::size_t count =
        std::min(size, maximumBytes);

    for (std::size_t index = 0; index < count; ++index) {
        if (index > 0) {
            output.write(static_cast<uint8_t>(' '));
        }

        char byte[3];
        std::snprintf(
            byte,
            sizeof(byte),
            "%02X",
            static_cast<unsigned int>(payload[index])
        );
        output.print(byte);
    }

    if (count < size) {
        output.print(" ...");
    }
}

} // namespace EventMonitorPayloadSafetyDetail


inline bool PrintStructuredEventPayload(
    Print& output,
    const uint8_t* payload,
    std::size_t size,
    const EventMonitorConfig& config
) noexcept {
    if (
        payload == nullptr ||
        size == 0
    ) {
        output.print("<none>");
        return true;
    }

    const auto limits =
        BuildEventMonitorDecodeLimits(config);

    // Validate first so malformed input never leaves a partially rendered
    // structured diagnostic line. Both passes are allocation-free.
    if (
        !Serializable::ValidateBinaryArchive(
            payload,
            size,
            limits
        )
    ) {
        return false;
    }

    EventMonitorPayloadSafetyDetail::
        StructuredPayloadPrinter visitor(
            output,
            config
        );

    return Serializable::TraverseBinaryArchive(
        payload,
        size,
        visitor,
        limits
    );
}


inline void PrintEventPayloadHexFallback(
    Print& output,
    const uint8_t* payload,
    std::size_t size,
    const EventMonitorConfig& config
) noexcept {
    EventMonitorPayloadSafetyDetail::PrintHexPayload(
        output,
        payload,
        size,
        config.MaximumHexPayloadBytes
    );
}

} // namespace ESPressio::Serial
