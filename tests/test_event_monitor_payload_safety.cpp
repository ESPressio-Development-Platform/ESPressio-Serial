#include <cassert>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include <Arduino.h>
#include <ESPressio_BinaryArchive.hpp>
#include <event/ESPressio_EventMonitorPayloadSafety.hpp>

using namespace ESPressio;

class BufferPrint final : public Print {
public:
    std::string Data;

    std::size_t write(uint8_t value) override {
        Data.push_back(static_cast<char>(value));
        return 1;
    }
};

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

    // Structured output is rendered directly from ESPB bytes and does not need
    // a second SerializationNode tree.
    {
        BufferPrint output;
        assert(Serial::PrintStructuredEventPayload(
            output,
            valid.data(),
            valid.size(),
            config
        ));
        assert(output.Data.find("\"value\"") != std::string::npos);
        assert(output.Data.find("42") != std::string::npos);
    }

    const auto deep = DeepPayload(16);
    assert(!Serial::ValidateStructuredEventPayload(
        deep.data(), deep.size(), config
    ));
    {
        BufferPrint output;
        assert(!Serial::PrintStructuredEventPayload(
            output,
            deep.data(),
            deep.size(),
            config
        ));
        // Validation runs before presentation, so rejected input never emits a
        // half-rendered structured payload.
        assert(output.Data.empty());
    }

    const std::vector<uint8_t> truncated = {
        'E', 'S', 'P', 'B', 2u,
        static_cast<uint8_t>(Serializable::SerializationNodeType::Object),
        1u, 0u
    };
    assert(!Serial::ValidateStructuredEventPayload(
        truncated.data(), truncated.size(), config
    ));

    // Stress the allocation-free diagnostic guard with deterministic arbitrary
    // byte sequences. The contract is bounded rejection without constructing
    // tree state or destabilizing the caller.
    std::mt19937 rng(0x45564D4Fu);
    std::vector<uint8_t> bytes(512);
    for (unsigned iteration = 0; iteration < 5000; ++iteration) {
        const std::size_t size = 1 + (rng() % bytes.size());
        for (std::size_t index = 0; index < size; ++index) {
            bytes[index] = static_cast<uint8_t>(rng());
        }
        (void)Serial::ValidateStructuredEventPayload(
            bytes.data(), size, config
        );
    }

    return 0;
}
