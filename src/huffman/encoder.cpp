#include "puff/huffman/encoder.hpp"

#include "puff/huffman/bit_writer.hpp"

namespace puff {

ByteFrequencyMap Encoder::calculate_frequencies(std::istream& input) {
    ByteFrequencyMap freq;
    char ch;
    while (input.get(ch)) {
        ++freq[static_cast<uint8_t>(ch)];
    }
    return freq;
}

EncodeResult Encoder::encode(std::istream& input,
                             std::ostream& output,
                             const HuffmanCodeMap& codes) {
    BitWriter writer(output);

    char ch;
    while (input.get(ch)) {
        auto it = codes.find(static_cast<uint8_t>(ch));
        if (it != codes.end()) {
            writer.write_bits(it->second);
        }
        // Symbols not in the code map are silently skipped — this should
        // never happen when the code map was built from the same data.
    }

    uint8_t padding = writer.flush();

    EncodeResult result;
    result.compressed_size = writer.bytes_written();
    result.padding_bits    = padding;
    return result;
}

}  // namespace puff
