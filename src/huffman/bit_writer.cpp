#include "puff/huffman/bit_writer.hpp"

namespace puff {

BitWriter::BitWriter(std::ostream& out)
    : out_(out) {}

void BitWriter::write_bit(bool bit) {
    // Shift the buffer left by 1 and insert the new bit at the LSB position.
    buffer_ = static_cast<uint8_t>((buffer_ << 1) | (bit ? 1 : 0));
    ++bit_count_;

    if (bit_count_ == 8) {
        out_.put(static_cast<char>(buffer_));
        ++bytes_written_;
        buffer_    = 0;
        bit_count_ = 0;
    }
}

void BitWriter::write_bits(const std::vector<bool>& bits) {
    for (bool b : bits) {
        write_bit(b);
    }
}

uint8_t BitWriter::flush() {
    if (bit_count_ == 0)
        return 0;

    uint8_t padding = static_cast<uint8_t>(8 - bit_count_);
    // Left-align the remaining bits in the byte.
    buffer_ = static_cast<uint8_t>(buffer_ << padding);
    out_.put(static_cast<char>(buffer_));
    ++bytes_written_;
    buffer_    = 0;
    bit_count_ = 0;
    return padding;
}

uint64_t BitWriter::bytes_written() const noexcept {
    return bytes_written_;
}

}  // namespace puff
