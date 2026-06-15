#include "puff/huffman/bit_reader.hpp"

namespace puff {

BitReader::BitReader(std::istream& in)
    : in_(in) {}

std::optional<bool> BitReader::read_bit() {
    if (bits_remaining_ == 0) {
        char ch;
        if (!in_.get(ch))
            return std::nullopt;  // EOF

        buffer_         = static_cast<uint8_t>(ch);
        bits_remaining_ = 8;
        ++bytes_read_;
    }

    --bits_remaining_;
    // Extract the most-significant remaining bit.
    bool bit = (buffer_ >> bits_remaining_) & 1;
    return bit;
}

uint64_t BitReader::bytes_read() const noexcept {
    return bytes_read_;
}

}  // namespace puff
