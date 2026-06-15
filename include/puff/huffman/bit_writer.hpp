#ifndef PUFF_HUFFMAN_BIT_WRITER_HPP
#define PUFF_HUFFMAN_BIT_WRITER_HPP

#include <cstdint>
#include <ostream>
#include <vector>

namespace puff {

/// Writes individual bits to a byte-oriented output stream.
///
/// Bits are accumulated in an internal byte buffer and flushed as complete
/// bytes.  Call flush() after all data has been written to emit any remaining
/// partial byte (zero-padded on the right).
class BitWriter {
public:
    /// Construct a BitWriter that writes to @p out.
    /// The stream must remain valid for the lifetime of this object.
    explicit BitWriter(std::ostream& out);

    /// Write a single bit (true = 1, false = 0).
    void write_bit(bool bit);

    /// Write a sequence of bits (e.g. a Huffman code).
    void write_bits(const std::vector<bool>& bits);

    /// Flush any remaining buffered bits (zero-padded to a full byte).
    /// @return The number of padding bits added (0–7).
    uint8_t flush();

    /// Total number of complete bytes written to the stream so far
    /// (does NOT include a partial byte still in the buffer).
    [[nodiscard]] uint64_t bytes_written() const noexcept;

private:
    std::ostream& out_;
    uint8_t  buffer_    = 0;    ///< Accumulates bits before writing a byte.
    int      bit_count_ = 0;    ///< Number of valid bits in buffer_ (0–7).
    uint64_t bytes_written_ = 0;
};

}  // namespace puff

#endif  // PUFF_HUFFMAN_BIT_WRITER_HPP
