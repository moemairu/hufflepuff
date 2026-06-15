#ifndef PUFF_HUFFMAN_HUFFMAN_TREE_HPP
#define PUFF_HUFFMAN_HUFFMAN_TREE_HPP

#include <cstdint>
#include <memory>

#include "puff/utils/types.hpp"

namespace puff {

// ── Node ────────────────────────────────────────────────────────────────────

/// A node in the Huffman tree.  Leaf nodes carry a symbol; internal nodes have
/// two children.  Managed via std::unique_ptr for automatic cleanup.
struct HuffmanNode {
    uint8_t  symbol    = 0;
    uint64_t frequency = 0;

    std::unique_ptr<HuffmanNode> left;
    std::unique_ptr<HuffmanNode> right;

    /// Construct a leaf node.
    HuffmanNode(uint8_t sym, uint64_t freq)
        : symbol(sym), frequency(freq) {}

    /// Construct an internal node by combining two children.
    HuffmanNode(std::unique_ptr<HuffmanNode> l, std::unique_ptr<HuffmanNode> r)
        : symbol(std::min(l->symbol, r->symbol)),
          frequency(l->frequency + r->frequency),
          left(std::move(l)),
          right(std::move(r)) {}

    [[nodiscard]] bool is_leaf() const noexcept {
        return !left && !right;
    }
};

// ── Tree ────────────────────────────────────────────────────────────────────

/// Builds and owns a Huffman tree from a byte-frequency map, and generates
/// the corresponding code table.
class HuffmanTree {
public:
    HuffmanTree() = default;

    /// Build the tree from a frequency map.
    /// @throws std::invalid_argument if frequencies is empty.
    void build(const ByteFrequencyMap& frequencies);

    /// Generate the variable-length code for every symbol in the tree.
    [[nodiscard]] HuffmanCodeMap generate_codes() const;

    /// Access the root (may be nullptr if tree hasn't been built).
    [[nodiscard]] const HuffmanNode* get_root() const noexcept;

    /// Check whether the tree has been built.
    [[nodiscard]] bool empty() const noexcept;

private:
    std::unique_ptr<HuffmanNode> root_;

    /// Recursive helper for generate_codes().
    static void generate_codes_impl(const HuffmanNode* node,
                                    std::vector<bool>& current_code,
                                    HuffmanCodeMap& codes);
};

}  // namespace puff

#endif  // PUFF_HUFFMAN_HUFFMAN_TREE_HPP
