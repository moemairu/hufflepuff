#include "test_harness.hpp"

#include "puff/huffman/decoder.hpp"
#include "puff/huffman/encoder.hpp"
#include "puff/huffman/huffman_tree.hpp"
#include "puff/utils/types.hpp"

#include <random>
#include <sstream>

// ── Helper ──────────────────────────────────────────────────────────────────

static void assert_roundtrip(const std::string& input) {
    // 1) Calculate frequencies.
    std::istringstream freq_in(input);
    puff::ByteFrequencyMap freq = puff::Encoder::calculate_frequencies(freq_in);

    // 2) Build tree and codes.
    puff::HuffmanTree tree;
    tree.build(freq);
    puff::HuffmanCodeMap codes = tree.generate_codes();

    // 3) Encode.
    std::istringstream encode_in(input);
    std::ostringstream encoded;
    puff::EncodeResult result = puff::Encoder::encode(encode_in, encoded, codes);

    // 4) Rebuild tree from same frequencies (simulates deserialization).
    puff::HuffmanTree tree2;
    tree2.build(freq);

    // 5) Decode.
    std::istringstream decode_in(encoded.str());
    std::ostringstream decoded;
    puff::Decoder::decode(decode_in, decoded, tree2,
                          input.size(), result.padding_bits);

    // 6) Assert byte-identical.
    ASSERT_EQ(decoded.str(), input);
}

// ── Tests ───────────────────────────────────────────────────────────────────

TEST_CASE("Encoder/Decoder: round-trip ASCII text") {
    assert_roundtrip("Hello, Hufflepuff! This is a test of the Huffman coding engine.");
}

TEST_CASE("Encoder/Decoder: round-trip single character") {
    assert_roundtrip("a");
}

TEST_CASE("Encoder/Decoder: round-trip repeated single character") {
    assert_roundtrip(std::string(1000, 'x'));
}

TEST_CASE("Encoder/Decoder: round-trip two characters") {
    assert_roundtrip("ababababab");
}

TEST_CASE("Encoder/Decoder: round-trip all 256 byte values") {
    std::string all_bytes;
    for (int i = 0; i < 256; ++i)
        all_bytes.push_back(static_cast<char>(i));
    assert_roundtrip(all_bytes);
}

TEST_CASE("Encoder/Decoder: round-trip binary data") {
    // Pseudorandom binary data.
    std::mt19937 rng(42);  // Fixed seed for reproducibility.
    std::uniform_int_distribution<int> dist(0, 255);

    std::string data;
    data.reserve(4096);
    for (int i = 0; i < 4096; ++i)
        data.push_back(static_cast<char>(dist(rng)));

    assert_roundtrip(data);
}

TEST_CASE("Encoder/Decoder: round-trip large text") {
    // Repeat a paragraph many times to simulate a large file.
    std::string paragraph =
        "The quick brown fox jumps over the lazy dog. "
        "Pack my box with five dozen liquor jugs. "
        "How vexingly quick daft zebras jump! ";

    std::string large;
    for (int i = 0; i < 500; ++i)
        large += paragraph;

    assert_roundtrip(large);
}

TEST_CASE("Encoder: calculate_frequencies correctness") {
    std::istringstream in("aabbbcccc");
    auto freq = puff::Encoder::calculate_frequencies(in);

    ASSERT_EQ(freq['a'], static_cast<uint64_t>(2));
    ASSERT_EQ(freq['b'], static_cast<uint64_t>(3));
    ASSERT_EQ(freq['c'], static_cast<uint64_t>(4));
    ASSERT_EQ(freq.size(), static_cast<size_t>(3));
}

TEST_CASE("Encoder: empty input produces empty frequency map") {
    std::istringstream in("");
    auto freq = puff::Encoder::calculate_frequencies(in);
    ASSERT_TRUE(freq.empty());
}
