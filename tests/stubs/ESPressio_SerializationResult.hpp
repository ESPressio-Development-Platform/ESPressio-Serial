#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ESPressio::Serializable {

enum class SerializationErrorCode : uint8_t {
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

enum class ValidationBehavior : uint8_t {
    FailFast,
    CollectAll
};

struct DeserializationOptions {
    ValidationBehavior Behavior =
        ValidationBehavior::CollectAll;

    std::size_t MaximumIssues = 64;
};

struct SerializationIssue {
    SerializationErrorCode Code =
        SerializationErrorCode::None;

    std::string Path;
    std::string Message;
};

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
