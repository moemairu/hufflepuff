#include "test_harness.hpp"

#include "puff/huffman/huffman_tree.hpp"
#include "puff/utils/types.hpp"

#include <set>

// ── Basic tree construction ─────────────────────────────────────────────────

TEST_CASE("HuffmanTree: build from simple frequencies") {
    puff::ByteFrequencyMap freq = {
        {'a', 5}, {'b', 9}, {'c', 12}, {'d', 13}, {'e', 16}, {'f', 45}
    };

    puff::HuffmanTree tree;
    tree.build(freq);

    ASSERT_FALSE(tree.empty());
    ASSERT_TRUE(tree.get_root() != nullptr);
    ASSERT_FALSE(tree.get_root()->is_leaf());
}

TEST_CASE("HuffmanTree: generate_codes produces prefix-free codes") {
    puff::ByteFrequencyMap freq = {
        {'a', 5}, {'b', 9}, {'c', 12}, {'d', 13}, {'e', 16}, {'f', 45}
    };

    puff::HuffmanTree tree;
    tree.build(freq);
    auto codes = tree.generate_codes();

    // Must have a code for each symbol.
    ASSERT_EQ(codes.size(), freq.size());

    // Verify prefix-free property: no code is a prefix of another.
    for (const auto& [sym1, code1] : codes) {
        for (const auto& [sym2, code2] : codes) {
            if (sym1 == sym2) continue;

            size_t min_len = std::min(code1.size(), code2.size());
            bool is_prefix = true;
            for (size_t i = 0; i < min_len; ++i) {
                if (code1[i] != code2[i]) {
                    is_prefix = false;
                    break;
                }
            }
            // A shorter code must not be a prefix of a longer one.
            if (code1.size() <= code2.size()) {
                ASSERT_FALSE(is_prefix);
            }
        }
    }
}

TEST_CASE("HuffmanTree: higher frequency → shorter or equal code") {
    puff::ByteFrequencyMap freq = {{'a', 1}, {'b', 100}};

    puff::HuffmanTree tree;
    tree.build(freq);
    auto codes = tree.generate_codes();

    ASSERT_TRUE(codes['b'].size() <= codes['a'].size());
}

// ── Edge cases ──────────────────────────────────────────────────────────────

TEST_CASE("HuffmanTree: single symbol") {
    puff::ByteFrequencyMap freq = {{'z', 42}};

    puff::HuffmanTree tree;
    tree.build(freq);
    auto codes = tree.generate_codes();

    ASSERT_EQ(codes.size(), static_cast<size_t>(1));
    // Single symbol should get code {0} (length 1).
    ASSERT_EQ(codes['z'].size(), static_cast<size_t>(1));
    ASSERT_EQ(codes['z'][0], false);
}

TEST_CASE("HuffmanTree: empty frequency map") {
    puff::ByteFrequencyMap freq;

    puff::HuffmanTree tree;
    tree.build(freq);

    ASSERT_TRUE(tree.empty());
    auto codes = tree.generate_codes();
    ASSERT_TRUE(codes.empty());
}

TEST_CASE("HuffmanTree: two symbols equal frequency") {
    puff::ByteFrequencyMap freq = {{'a', 10}, {'b', 10}};

    puff::HuffmanTree tree;
    tree.build(freq);
    auto codes = tree.generate_codes();

    ASSERT_EQ(codes.size(), static_cast<size_t>(2));
    // Both codes should have length 1.
    ASSERT_EQ(codes['a'].size(), static_cast<size_t>(1));
    ASSERT_EQ(codes['b'].size(), static_cast<size_t>(1));
    // They must be different.
    ASSERT_TRUE(codes['a'] != codes['b']);
}

TEST_CASE("HuffmanTree: all 256 byte values") {
    puff::ByteFrequencyMap freq;
    for (int i = 0; i < 256; ++i)
        freq[static_cast<uint8_t>(i)] = static_cast<uint64_t>(i + 1);

    puff::HuffmanTree tree;
    tree.build(freq);
    auto codes = tree.generate_codes();

    ASSERT_EQ(codes.size(), static_cast<size_t>(256));

    // All codes must be non-empty.
    for (const auto& [sym, code] : codes) {
        ASSERT_TRUE(!code.empty());
    }
}
