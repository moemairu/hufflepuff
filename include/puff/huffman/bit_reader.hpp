#ifndef PUFF_HUFFMAN_BIT_READER_HPP
#define PUFF_HUFFMAN_BIT_READER_HPP

#include <cstdint>
#include <istream>
#include <optional>

namespace puff {

/// Reads individual bits from a byte-oriented input stream.
///
/// Internally buffers one byte at a time and delivers bits MSB-first
/// (matching the order produced by BitWriter).
class BitReader {
public:
    /// Construct a BitReader that reads from @p in.
    /// The stream must remain valid for the lifetime of this object.
    explicit BitReader(std::istream& in);

    /// Read a single bit.
    /// @return The bit value, or std::nullopt when the stream is exhausted.
    std::optional<bool> read_bit();

    /// Total number of complete bytes consumed from the stream so far.
    [[nodiscard]] uint64_t bytes_read() const noexcept;

private:
    std::istream& in_;
    uint8_t  buffer_         = 0;
    int      bits_remaining_ = 0;   ///< Valid bits left in buffer_ (0 = need refill).
    uint64_t bytes_read_     = 0;
};

}  // namespace puff

#endif  // PUFF_HUFFMAN_BIT_READER_HPP
