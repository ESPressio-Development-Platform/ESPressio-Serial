#pragma once

#include <algorithm>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <string_view>

#include <Arduino.h>
#include <ESPressio_BinaryArchive.hpp>
#include <ESPressio_EventTransport.hpp>

#include "../ESPressio_SerialTypes.hpp"

namespace ESPressio::Serial {

class EventMonitorFormatter final {
private:
    static const char* StageName(
        Event::EventTransportTransactionStage stage
    ) noexcept {
        using Stage =
            Event::EventTransportTransactionStage;

        switch (stage) {
            case Stage::OutboundAccepted:
                return "OutboundAccepted";
            case Stage::OutboundSerialized:
                return "OutboundSerialized";
            case Stage::OutboundHandedToTransport:
                return "OutboundHandedToTransport";
            case Stage::InboundAccepted:
                return "InboundAccepted";
            case Stage::InboundRejected:
                return "InboundRejected";
            case Stage::InboundDeserialized:
                return "InboundDeserialized";
            case Stage::InboundDispatched:
                return "InboundDispatched";
            case Stage::Failed:
                return "Failed";
        }

        return "Unknown";
    }


    static const char* DirectionName(
        Event::EventTransportDirection direction
    ) noexcept {
        using Direction =
            Event::EventTransportDirection;

        switch (direction) {
            case Direction::None:
                return "None";
            case Direction::Inbound:
                return "IN";
            case Direction::Outbound:
                return "OUT";
            case Direction::Bidirectional:
                return "IN/OUT";
        }

        return "?";
    }


    static const char* DispatchMethodName(
        Event::EventDispatchMethod method
    ) noexcept {
        switch (method) {
            case Event::EventDispatchMethod::Stack:
                return "Stack";
            case Event::EventDispatchMethod::Queue:
                return "Queue";
        }

        return "?";
    }


    static const char* PriorityName(
        Event::EventPriority priority
    ) noexcept {
        switch (priority) {
            case Event::EventPriority::Low:
                return "Low";
            case Event::EventPriority::Normal:
                return "Normal";
            case Event::EventPriority::High:
                return "High";
        }

        return "?";
    }


    static const char* OriginName(
        Event::EventOrigin origin
    ) noexcept {
        return
            origin == Event::EventOrigin::Remote
                ? "Remote"
                : "Local";
    }


    static void PrintUnsigned64(
        Print& output,
        uint64_t value
    ) {
        char buffer[32];

        std::snprintf(
            buffer,
            sizeof(buffer),
            "%" PRIu64,
            value
        );

        output.print(buffer);
    }


    static void PrintHex64(
        Print& output,
        uint64_t value
    ) {
        char buffer[32];

        std::snprintf(
            buffer,
            sizeof(buffer),
            "0x%016" PRIX64,
            value
        );

        output.print(buffer);
    }


    static void PrintPointer(
        Print& output,
        const void* pointer
    ) {
        char buffer[
            2 + sizeof(uintptr_t) * 2 + 1
        ];

        std::snprintf(
            buffer,
            sizeof(buffer),
            "0x%" PRIXPTR,
            reinterpret_cast<uintptr_t>(
                pointer
            )
        );

        output.print(buffer);
    }


    static void PrintIndent(
        Print& output,
        uint8_t depth,
        uint8_t spaces
    ) {
        const std::size_t count =
            static_cast<std::size_t>(depth) *
            spaces;

        for (
            std::size_t index = 0;
            index < count;
            ++index
        ) {
            output.write(
                static_cast<uint8_t>(' ')
            );
        }
    }


    static void PrintEscapedString(
        Print& output,
        std::string_view value,
        std::size_t maximumLength
    ) {
        output.write(
            static_cast<uint8_t>('"')
        );

        const std::size_t length =
            std::min(
                value.size(),
                maximumLength
            );

        for (
            std::size_t index = 0;
            index < length;
            ++index
        ) {
            const unsigned char character =
                static_cast<unsigned char>(
                    value[index]
                );

            switch (character) {
                case '"':
                    output.print("\\\"");
                    break;

                case '\\':
                    output.print("\\\\");
                    break;

                case '\b':
                    output.print("\\b");
                    break;

                case '\f':
                    output.print("\\f");
                    break;

                case '\n':
                    output.print("\\n");
                    break;

                case '\r':
                    output.print("\\r");
                    break;

                case '\t':
                    output.print("\\t");
                    break;

                default:
                    if (character < 0x20u) {
                        char escaped[7];

                        std::snprintf(
                            escaped,
                            sizeof(escaped),
                            "\\u%04X",
                            static_cast<unsigned int>(
                                character
                            )
                        );

                        output.print(
                            escaped
                        );
                    } else {
                        output.write(
                            static_cast<uint8_t>(
                                character
                            )
                        );
                    }

                    break;
            }
        }

        if (
            value.size() >
            maximumLength
        ) {
            output.print("...");
        }

        output.write(
            static_cast<uint8_t>('"')
        );
    }


    static void PrintNode(
        Print& output,
        const Serializable::SerializationNode& node,
        const EventMonitorConfig& config,
        uint8_t depth
    ) {
        using NodeType =
            Serializable::SerializationNodeType;

        if (
            depth >
            config.MaximumStructuredDepth
        ) {
            output.print("\"<max-depth>\"");
            return;
        }

        switch (node.GetType()) {
            case NodeType::Null:
                output.print("null");
                return;

            case NodeType::Boolean:
                output.print(
                    node.BooleanValue()
                        ? "true"
                        : "false"
                );
                return;

            case NodeType::SignedInteger: {
                char buffer[32];

                std::snprintf(
                    buffer,
                    sizeof(buffer),
                    "%" PRId64,
                    node.SignedIntegerValue()
                );

                output.print(buffer);
                return;
            }

            case NodeType::UnsignedInteger: {
                char buffer[32];

                std::snprintf(
                    buffer,
                    sizeof(buffer),
                    "%" PRIu64,
                    node.UnsignedIntegerValue()
                );

                output.print(buffer);
                return;
            }

            case NodeType::Float32:
                output.print(
                    node.Float32Value(),
                    7
                );
                return;

            case NodeType::Float64:
                output.print(
                    node.Float64Value(),
                    15
                );
                return;

            case NodeType::String:
                PrintEscapedString(
                    output,
                    node.StringValue(),
                    config.MaximumStringLength
                );
                return;

            case NodeType::Object: {
                const auto& children =
                    node.ObjectChildren();

                output.write(
                    static_cast<uint8_t>('{')
                );

                const std::size_t count =
                    std::min(
                        children.size(),
                        config.MaximumCollectionItems
                    );

                for (
                    std::size_t index = 0;
                    index < count;
                    ++index
                ) {
                    if (index > 0) {
                        output.write(
                            static_cast<uint8_t>(',')
                        );
                    }

                    if (
                        config.PrettyStructuredPayload
                    ) {
                        output.println();

                        PrintIndent(
                            output,
                            static_cast<uint8_t>(
                                depth + 1
                            ),
                            config.IndentSpaces
                        );
                    }

                    PrintEscapedString(
                        output,
                        children[index].first,
                        config.MaximumStringLength
                    );

                    output.print(
                        config.PrettyStructuredPayload
                            ? ": "
                            : ":"
                    );

                    PrintNode(
                        output,
                        children[index].second,
                        config,
                        static_cast<uint8_t>(
                            depth + 1
                        )
                    );
                }

                if (
                    children.size() >
                    count
                ) {
                    if (count > 0) {
                        output.write(
                            static_cast<uint8_t>(',')
                        );
                    }

                    if (
                        config.PrettyStructuredPayload
                    ) {
                        output.println();

                        PrintIndent(
                            output,
                            static_cast<uint8_t>(
                                depth + 1
                            ),
                            config.IndentSpaces
                        );
                    }

                    output.print(
                        "\"...\":\"<truncated>\""
                    );
                }

                if (
                    config.PrettyStructuredPayload &&
                    !children.empty()
                ) {
                    output.println();

                    PrintIndent(
                        output,
                        depth,
                        config.IndentSpaces
                    );
                }

                output.write(
                    static_cast<uint8_t>('}')
                );
                return;
            }

            case NodeType::Array: {
                const auto& children =
                    node.ArrayChildren();

                output.write(
                    static_cast<uint8_t>('[')
                );

                const std::size_t count =
                    std::min(
                        children.size(),
                        config.MaximumCollectionItems
                    );

                for (
                    std::size_t index = 0;
                    index < count;
                    ++index
                ) {
                    if (index > 0) {
                        output.write(
                            static_cast<uint8_t>(',')
                        );
                    }

                    if (
                        config.PrettyStructuredPayload
                    ) {
                        output.println();

                        PrintIndent(
                            output,
                            static_cast<uint8_t>(
                                depth + 1
                            ),
                            config.IndentSpaces
                        );
                    }

                    PrintNode(
                        output,
                        children[index],
                        config,
                        static_cast<uint8_t>(
                            depth + 1
                        )
                    );
                }

                if (
                    children.size() >
                    count
                ) {
                    if (count > 0) {
                        output.write(
                            static_cast<uint8_t>(',')
                        );
                    }

                    if (
                        config.PrettyStructuredPayload
                    ) {
                        output.println();

                        PrintIndent(
                            output,
                            static_cast<uint8_t>(
                                depth + 1
                            ),
                            config.IndentSpaces
                        );
                    }

                    output.print(
                        "\"<truncated>\""
                    );
                }

                if (
                    config.PrettyStructuredPayload &&
                    !children.empty()
                ) {
                    output.println();

                    PrintIndent(
                        output,
                        depth,
                        config.IndentSpaces
                    );
                }

                output.write(
                    static_cast<uint8_t>(']')
                );
                return;
            }
        }
    }


    static void PrintHexPayload(
        Print& output,
        const uint8_t* payload,
        std::size_t size,
        std::size_t maximumBytes
    ) {
        if (
            payload == nullptr ||
            size == 0
        ) {
            output.print("<none>");
            return;
        }

        const std::size_t count =
            std::min(
                size,
                maximumBytes
            );

        for (
            std::size_t index = 0;
            index < count;
            ++index
        ) {
            if (index > 0) {
                output.write(
                    static_cast<uint8_t>(' ')
                );
            }

            char byte[3];

            std::snprintf(
                byte,
                sizeof(byte),
                "%02X",
                static_cast<unsigned int>(
                    payload[index]
                )
            );

            output.print(byte);
        }

        if (count < size) {
            output.print(" ...");
        }
    }


    static void PrintStructuredPayload(
        Print& output,
        const uint8_t* payload,
        std::size_t size,
        const EventMonitorConfig& config
    ) {
        if (
            payload == nullptr ||
            size == 0
        ) {
            output.print("<none>");
            return;
        }

        Serializable::BinaryArchive archive;

        if (
            !archive.Load(
                payload,
                size
            )
        ) {
            output.print(
                "<invalid-binary-archive> "
            );

            PrintHexPayload(
                output,
                payload,
                size,
                config.MaximumHexPayloadBytes
            );

            return;
        }

        PrintNode(
            output,
            archive.GetNode(),
            config,
            0
        );
    }


public:
    static bool IsTransactionVisible(
        const Event::EventTransportTransaction&
            transaction,
        const EventMonitorConfig& config
    ) noexcept {
        const bool inbound =
            transaction.Direction ==
            Event::EventTransportDirection::
                Inbound;

        const bool outbound =
            transaction.Direction ==
            Event::EventTransportDirection::
                Outbound;

        if (
            inbound &&
            !config.ShowInbound
        ) {
            return false;
        }

        if (
            outbound &&
            !config.ShowOutbound
        ) {
            return false;
        }

        if (
            !config.ShowFailures &&
            (
                transaction.Stage ==
                    Event::EventTransportTransactionStage::
                        Failed ||
                transaction.Stage ==
                    Event::EventTransportTransactionStage::
                        InboundRejected
            )
        ) {
            return false;
        }

        if (
            config.Mode ==
            EventMonitorMode::Lifecycle
        ) {
            return true;
        }

        using Stage =
            Event::EventTransportTransactionStage;

        return
            transaction.Stage ==
                Stage::
                    OutboundHandedToTransport ||
            transaction.Stage ==
                Stage::
                    InboundDeserialized ||
            transaction.Stage ==
                Stage::
                    InboundRejected ||
            transaction.Stage ==
                Stage::Failed;
    }


    static void PrintTransaction(
        Print& output,
        const Event::EventTransportTransaction&
            transaction,
        const EventMonitorConfig& config
    ) {
        output.print("[ESPressio Event] [");
        output.print(
            DirectionName(
                transaction.Direction
            )
        );
        output.print("] [");
        output.print(
            StageName(
                transaction.Stage
            )
        );
        output.print("]");

        if (
            config.ShowStableTypeName
        ) {
            output.print(" type=");

            if (
                transaction.EventTypeName.empty()
            ) {
                output.print("<unknown>");
            } else {
                output.write(
                    reinterpret_cast<
                        const uint8_t*
                    >(
                        transaction.
                            EventTypeName.data()
                    ),
                    transaction.
                        EventTypeName.size()
                );
            }
        }

        if (config.ShowTypeID) {
            output.print(" typeId=");
            PrintHex64(
                output,
                transaction.EventTypeID
            );
        }

        if (config.ShowSchemaVersion) {
            output.print(" schema=");
            output.print(
                transaction.SchemaVersion
            );
        }

        if (config.ShowMessageID) {
            output.print(" message=");
            PrintUnsigned64(
                output,
                transaction.MessageID
            );
        }

        if (
            config.ShowTransportAddress
        ) {
            output.print(" transport=");
            PrintPointer(
                output,
                transaction.Transport
            );
        }

        if (
            config.ShowDispatchMetadata
        ) {
            output.print(" dispatch=");
            output.print(
                DispatchMethodName(
                    transaction.DispatchMethod
                )
            );

            output.print(" priority=");
            output.print(
                PriorityName(
                    transaction.Priority
                )
            );
        }

        if (config.ShowOrigin) {
            output.print(" origin=");
            output.print(
                OriginName(
                    transaction.Origin
                )
            );
        }

        if (config.ShowHopCount) {
            output.print(" hops=");
            output.print(
                transaction.HopCount
            );
        }

        if (
            config.ShowTransportAccepted &&
            transaction.Stage ==
                Event::EventTransportTransactionStage::
                    OutboundHandedToTransport
        ) {
            output.print(" accepted=");
            output.print(
                transaction.TransportAccepted
                    ? "true"
                    : "false"
            );
        }

        output.print(" payloadBytes=");
        output.println(
            transaction.PayloadSize
        );

        switch (config.PayloadFormat) {
            case EventMonitorPayloadFormat::None:
                break;

            case EventMonitorPayloadFormat::Summary:
                output.print("  payload: <");
                output.print(
                    transaction.PayloadSize
                );
                output.println(" bytes>");
                break;

            case EventMonitorPayloadFormat::Hex:
                output.print("  payload: ");

                PrintHexPayload(
                    output,
                    transaction.Payload,
                    transaction.PayloadSize,
                    config.MaximumHexPayloadBytes
                );

                output.println();
                break;

            case EventMonitorPayloadFormat::Structured:
                output.print("  payload: ");

                PrintStructuredPayload(
                    output,
                    transaction.Payload,
                    transaction.PayloadSize,
                    config
                );

                output.println();
                break;
        }
    }
};

}
