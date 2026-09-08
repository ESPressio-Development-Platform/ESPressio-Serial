#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ESPressio::Serializable {

/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
enum
class SerializationErrorCode : uint8_t {
    None,
    MissingRequiredProperty,
    TypeMismatch,
    NumericOutOfRange,
    ValidationFailed,
    UnsupportedSchemaVersion,
    MigrationFailed,
    MalformedInput,
    StreamError,
    DuplicateValue,
    UnknownEnumValue,
    ResourceLimitExceeded
};

/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
enum
class ValidationBehavior : uint8_t {
    FailFast,
    CollectAll
};

/**
 * ESPressio Memory Audit
 * Members:
 * - Behavior (ValidationBehavior): 1 bytes [0 bytes dynamic allocation]
 * - MaximumIssues (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 8 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct DeserializationOptions {
    ValidationBehavior Behavior =
        ValidationBehavior::CollectAll;

    std::size_t MaximumIssues = 64;
};

/**
 * ESPressio Memory Audit
 * Members:
 * - Code (SerializationErrorCode): 1 bytes [0 bytes dynamic allocation]
 * - Path (std::string): 24 bytes [Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * - Message (std::string): 24 bytes [Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Total Memory: 52 bytes [Path: Capacity + 1 bytes when capacity exceeds 15-byte SSO; Message: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
struct SerializationIssue {
    SerializationErrorCode Code =
        SerializationErrorCode::None;

    std::string Path;
    std::string Message;
};

/**
 * ESPressio Memory Audit
 * Members:
 * - _issues (std::vector<SerializationIssue>): 12 bytes [Capacity * (52 bytes) element storage; N live elements each: Path: Capacity + 1 bytes when capacity exceeds 15-byte SSO; N live elements each: Message: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Total Memory: 12 bytes [_issues: Capacity * (52 bytes) element storage; _issues: N live elements each: Path: Capacity + 1 bytes when capacity exceeds 15-byte SSO; _issues: N live elements each: Message: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class DeserializationResult {
private:
    std::vector<SerializationIssue>
        _issues;

public:
    bool Success() const {
        return _issues.empty();
    }

    std::size_t IssueCount() const {
        return _issues.size();
    }

    const auto& Issues() const {
        return _issues;
    }
};

}
