#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

#include <ESPressio_BinaryArchive.hpp>
#include <event/ESPressio_EventMonitorPayloadSafety.hpp>

using namespace ESPressio;

static void AppendU16(std::vector<uint8_t>& data, uint16_t value) {
    data.push_back(static_cast<uint8_t>(value & 0xffu));
    data.push_back(static_cast<uint8_t>((value >> 8u) & 0xffu));
}

static std::vector<uint8_t> DeepPayload(unsigned depth) {
    std::vector<uint8_t> data = {'E', 'S', 'P', 'B', 2u};
    for (unsigned level = 0; level < depth; ++level) {
        data.push_back(static_cast<uint8_t>(
            Serializable::SerializationNodeType::Object
        ));
        AppendU16(data, 1);
        AppendU16(data, 1);
        data.push_back('x');
    }
    data.push_back(static_cast<uint8_t>(
        Serializable::SerializationNodeType::Null
    ));
    return data;
}

int main() {
    Serial::EventMonitorConfig config;
    config.MaximumStructuredDepth = 8;
    config.MaximumStructuredNodes = 128;
    config.MaximumCollectionItems = 32;
    config.MaximumStringLength = 128;

    Serializable::BinaryArchive validArchive;
    validArchive.Write("value", uint32_t(42));
    const auto valid = validArchive.GetData();
    assert(Serial::ValidateStructuredEventPayload(
        valid.data(), valid.size(), config
    ));

    const auto deep = DeepPayload(16);
    assert(!Serial::ValidateStructuredEventPayload(
        deep.data(), deep.size(), config
    ));

    const std::vector<uint8_t> truncated = {
        'E', 'S', 'P', 'B', 2u,
        static_cast<uint8_t>(Serializable::SerializationNodeType::Object),
        1u, 0u
    };
    assert(!Serial::ValidateStructuredEventPayload(
        truncated.data(), truncated.size(), config
    ));

    // Stress the diagnostic guard with deterministic arbitrary byte sequences.
    // The contract is not that random data becomes valid; it is that validation
    // remains bounded and never destabilizes the caller.
    std::mt19937 rng(0x45564D4Fu);
    for (unsigned iteration = 0; iteration < 5000; ++iteration) {
        const std::size_t size = 1 + (rng() % 512);
        std::vector<uint8_t> bytes(size);
        for (auto& byte : bytes) {
            byte = static_cast<uint8_t>(rng());
        }
        (void)Serial::ValidateStructuredEventPayload(
            bytes.data(), bytes.size(), config
        );
    }

    return 0;
}
