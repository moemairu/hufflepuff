#ifndef PUFF_UTILS_CRC32_HPP
#define PUFF_UTILS_CRC32_HPP

#include <cstddef>
#include <cstdint>

namespace puff {

/// Initialize a CRC-32 accumulator.
[[nodiscard]] uint32_t crc32_init() noexcept;

/// Feed bytes into a running CRC-32 computation.
[[nodiscard]] uint32_t crc32_update(uint32_t crc,
                                    const uint8_t* data,
                                    size_t length) noexcept;

/// Finalize the CRC-32 value.
[[nodiscard]] uint32_t crc32_finalize(uint32_t crc) noexcept;

/// Convenience: compute CRC-32 of a contiguous buffer in one call.
[[nodiscard]] uint32_t crc32(const uint8_t* data, size_t length) noexcept;

}  // namespace puff

#endif  // PUFF_UTILS_CRC32_HPP
