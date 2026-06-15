#include "puff/huffman/decoder.hpp"

#include "puff/huffman/bit_reader.hpp"

#include <stdexcept>

namespace puff {

void Decoder::decode(std::istream& input,
                     std::ostream& output,
                     const HuffmanTree& tree,
                     uint64_t original_size,
                     uint8_t /*padding_bits*/) {
    if (original_size == 0 || tree.empty())
        return;

    const HuffmanNode* root = tree.get_root();
    BitReader reader(input);

    // Special case: single-symbol tree (root is a leaf).
    if (root->is_leaf()) {
        for (uint64_t i = 0; i < original_size; ++i) {
            output.put(static_cast<char>(root->symbol));
            // Consume the corresponding bit from the stream.
            reader.read_bit();
        }
        return;
    }

    uint64_t decoded = 0;
    const HuffmanNode* current = root;

    while (decoded < original_size) {
        auto bit = reader.read_bit();
        if (!bit.has_value()) {
            throw std::runtime_error(
                "Unexpected end of bitstream during decoding "
                "(decoded " + std::to_string(decoded) + " of " +
                std::to_string(original_size) + " bytes)");
        }

        current = bit.value() ? current->right.get() : current->left.get();

        if (!current) {
            throw std::runtime_error("Invalid Huffman tree path encountered during decoding");
        }

        if (current->is_leaf()) {
            output.put(static_cast<char>(current->symbol));
            ++decoded;
            current = root;
        }
    }
}

}  // namespace puff
