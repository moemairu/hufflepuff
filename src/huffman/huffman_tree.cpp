#include "puff/huffman/huffman_tree.hpp"

#include <functional>
#include <queue>
#include <stdexcept>

namespace puff {

// ── Build ───────────────────────────────────────────────────────────────────

void HuffmanTree::build(const ByteFrequencyMap& frequencies) {
    if (frequencies.empty()) {
        root_ = nullptr;
        return;
    }

    // Comparator for the min-heap.  When frequencies are equal, the node with
    // the smaller symbol value wins — this guarantees a deterministic tree so
    // the encoder and decoder always agree.
    auto cmp = [](const std::unique_ptr<HuffmanNode>& a,
                  const std::unique_ptr<HuffmanNode>& b) {
        if (a->frequency != b->frequency)
            return a->frequency > b->frequency;  // min-heap: smaller freq = higher priority
        return a->symbol > b->symbol;             // tie-break: smaller symbol = higher priority
    };

    // Use a vector-backed priority queue.
    std::priority_queue<std::unique_ptr<HuffmanNode>,
                        std::vector<std::unique_ptr<HuffmanNode>>,
                        decltype(cmp)> pq(cmp);

    for (const auto& [sym, freq] : frequencies) {
        pq.push(std::make_unique<HuffmanNode>(sym, freq));
    }

    // Merge nodes until only the root remains.
    while (pq.size() > 1) {
        // We need to const_cast + move because std::priority_queue::top()
        // returns a const ref, and there is no "pop-and-move" API.
        auto left  = std::move(const_cast<std::unique_ptr<HuffmanNode>&>(pq.top()));
        pq.pop();
        auto right = std::move(const_cast<std::unique_ptr<HuffmanNode>&>(pq.top()));
        pq.pop();

        pq.push(std::make_unique<HuffmanNode>(std::move(left), std::move(right)));
    }

    root_ = std::move(const_cast<std::unique_ptr<HuffmanNode>&>(pq.top()));
    pq.pop();
}

// ── Code generation ─────────────────────────────────────────────────────────

HuffmanCodeMap HuffmanTree::generate_codes() const {
    HuffmanCodeMap codes;
    if (!root_) return codes;

    std::vector<bool> current_code;

    // Edge case: single-symbol tree (root is a leaf).
    if (root_->is_leaf()) {
        codes[root_->symbol] = {false};  // Assign code "0"
        return codes;
    }

    generate_codes_impl(root_.get(), current_code, codes);
    return codes;
}

void HuffmanTree::generate_codes_impl(const HuffmanNode* node,
                                      std::vector<bool>& current_code,
                                      HuffmanCodeMap& codes) {
    if (!node) return;

    if (node->is_leaf()) {
        codes[node->symbol] = current_code;
        return;
    }

    current_code.push_back(false);  // left  = 0
    generate_codes_impl(node->left.get(), current_code, codes);
    current_code.pop_back();

    current_code.push_back(true);   // right = 1
    generate_codes_impl(node->right.get(), current_code, codes);
    current_code.pop_back();
}

// ── Accessors ───────────────────────────────────────────────────────────────

const HuffmanNode* HuffmanTree::get_root() const noexcept {
    return root_.get();
}

bool HuffmanTree::empty() const noexcept {
    return root_ == nullptr;
}

}  // namespace puff
