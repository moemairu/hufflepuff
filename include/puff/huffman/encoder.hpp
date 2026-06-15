#ifndef PUFF_HUFFMAN_ENCODER_HPP
#define PUFF_HUFFMAN_ENCODER_HPP

#include <istream>
#include <ostream>

#include "puff/utils/types.hpp"

namespace puff {

/// Calculates byte frequencies and Huffman-encodes a stream of bytes.
class Encoder {
public:
    /// Scan the entire input stream and count the frequency of each byte value.
    /// The stream's read position is consumed; the caller should reset it if
    /// the same stream will be read again.
    static ByteFrequencyMap calculate_frequencies(std::istream& input);

    /// Encode @p input into @p output using the provided Huffman codes.
    ///
    /// @param input   The raw byte stream to compress.
    /// @param output  The destination for the packed bitstream.
    /// @param codes   A Huffman code map (symbol → bit sequence).
    /// @return An EncodeResult with the compressed size and padding count.
    static EncodeResult encode(std::istream& input,
                               std::ostream& output,
                               const HuffmanCodeMap& codes);
};

}  // namespace puff

#endif  // PUFF_HUFFMAN_ENCODER_HPP
