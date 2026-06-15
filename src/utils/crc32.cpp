#include "puff/utils/crc32.hpp"

#include <array>

namespace puff {

namespace {

/// Generate the standard CRC-32 lookup table at compile time.
constexpr std::array<uint32_t, 256> make_crc_table() {
    constexpr uint32_t polynomial = 0xEDB88320u;
    std::array<uint32_t, 256> table{};

    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            if (crc & 1)
                crc = (crc >> 1) ^ polynomial;
            else
                crc >>= 1;
        }
        table[i] = crc;
    }
    return table;
}

constexpr auto crc_table = make_crc_table();

}  // anonymous namespace

uint32_t crc32_init() noexcept {
    return 0xFFFFFFFFu;
}

uint32_t crc32_update(uint32_t crc,
                      const uint8_t* data,
                      size_t length) noexcept {
    for (size_t i = 0; i < length; ++i) {
        uint8_t index = static_cast<uint8_t>(crc ^ data[i]);
        crc = (crc >> 8) ^ crc_table[index];
    }
    return crc;
}

uint32_t crc32_finalize(uint32_t crc) noexcept {
    return crc ^ 0xFFFFFFFFu;
}

uint32_t crc32(const uint8_t* data, size_t length) noexcept {
    return crc32_finalize(crc32_update(crc32_init(), data, length));
}

}  // namespace puff
