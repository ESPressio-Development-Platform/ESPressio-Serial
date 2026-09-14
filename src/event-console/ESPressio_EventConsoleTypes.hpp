#pragma once

#include <cstddef>

namespace ESPressio::Serial {

/// Operator-facing bounds for descriptor-driven Event tooling.
/// Family schema bounds remain authoritative; this value may only tighten them.
struct EventConsoleConfig final {
    std::size_t MaximumJsonLength = 2048;
};

} // namespace ESPressio::Serial
