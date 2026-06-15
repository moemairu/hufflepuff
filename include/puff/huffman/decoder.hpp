#ifndef PUFF_HUFFMAN_DECODER_HPP
#define PUFF_HUFFMAN_DECODER_HPP

#include <cstdint>
#include <istream>
#include <ostream>

#include "puff/huffman/huffman_tree.hpp"

namespace puff {

/// Decodes a Huffman-encoded bitstream back into the original byte data.
class Decoder {
public:
    /// Decode the bitstream in @p input, writing the original bytes to @p output.
    ///
    /// @param input          Compressed bitstream.
    /// @param output         Destination for decoded bytes.
    /// @param tree           The Huffman tree used during encoding.
    /// @param original_size  Expected number of decoded bytes (used to know
    ///                       when to stop — avoids interpreting padding bits).
    /// @param padding_bits   Number of padding bits in the last byte (0–7).
    static void decode(std::istream& input,
                       std::ostream& output,
                       const HuffmanTree& tree,
                       uint64_t original_size,
                       uint8_t padding_bits);
};

}  // namespace puff

#endif  // PUFF_HUFFMAN_DECODER_HPP
