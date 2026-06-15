#include "test_harness.hpp"

#include "puff/huffman/bit_reader.hpp"
#include "puff/huffman/bit_writer.hpp"

#include <sstream>

// ── BitWriter/BitReader round-trip ──────────────────────────────────────────

TEST_CASE("BitIO: write and read single bits") {
    std::ostringstream out;

    {
        puff::BitWriter writer(out);
        // Write 8 bits: 10110010
        writer.write_bit(true);
        writer.write_bit(false);
        writer.write_bit(true);
        writer.write_bit(true);
        writer.write_bit(false);
        writer.write_bit(false);
        writer.write_bit(true);
        writer.write_bit(false);

        uint8_t padding = writer.flush();
        ASSERT_EQ(static_cast<int>(padding), 0);  // Exactly 8 bits, no padding.
    }

    std::string data = out.str();
    ASSERT_EQ(data.size(), static_cast<size_t>(1));
    ASSERT_EQ(static_cast<uint8_t>(data[0]), static_cast<uint8_t>(0b10110010));

    // Read back.
    std::istringstream in(data);
    puff::BitReader reader(in);

    auto b0 = reader.read_bit(); ASSERT_TRUE(b0.has_value()); ASSERT_EQ(*b0, true);
    auto b1 = reader.read_bit(); ASSERT_TRUE(b1.has_value()); ASSERT_EQ(*b1, false);
    auto b2 = reader.read_bit(); ASSERT_TRUE(b2.has_value()); ASSERT_EQ(*b2, true);
    auto b3 = reader.read_bit(); ASSERT_TRUE(b3.has_value()); ASSERT_EQ(*b3, true);
    auto b4 = reader.read_bit(); ASSERT_TRUE(b4.has_value()); ASSERT_EQ(*b4, false);
    auto b5 = reader.read_bit(); ASSERT_TRUE(b5.has_value()); ASSERT_EQ(*b5, false);
    auto b6 = reader.read_bit(); ASSERT_TRUE(b6.has_value()); ASSERT_EQ(*b6, true);
    auto b7 = reader.read_bit(); ASSERT_TRUE(b7.has_value()); ASSERT_EQ(*b7, false);

    // Next read should be EOF.
    auto b8 = reader.read_bit();
    ASSERT_FALSE(b8.has_value());
}

TEST_CASE("BitIO: write_bits vector") {
    std::ostringstream out;

    {
        puff::BitWriter writer(out);
        // Write a 5-bit code: 11010
        writer.write_bits({true, true, false, true, false});
        uint8_t padding = writer.flush();
        ASSERT_EQ(static_cast<int>(padding), 3);  // 5 bits → 3 padding.
    }

    std::string data = out.str();
    ASSERT_EQ(data.size(), static_cast<size_t>(1));
    // 11010 + 000 (padding) = 11010000 = 0xD0
    ASSERT_EQ(static_cast<uint8_t>(data[0]), static_cast<uint8_t>(0xD0));
}

TEST_CASE("BitIO: multi-byte round-trip") {
    // Write 20 bits: 10101010 11001100 1111xxxx
    std::vector<bool> bits = {
        true, false, true, false, true, false, true, false,   // 0xAA
        true, true, false, false, true, true, false, false,   // 0xCC
        true, true, true, true                                // 0xF0 (padded)
    };

    std::ostringstream out;
    {
        puff::BitWriter writer(out);
        writer.write_bits(bits);
        uint8_t padding = writer.flush();
        ASSERT_EQ(static_cast<int>(padding), 4);  // 20 bits → 4 padding.
    }

    std::string data = out.str();
    ASSERT_EQ(data.size(), static_cast<size_t>(3));

    // Read back.
    std::istringstream in(data);
    puff::BitReader reader(in);

    for (size_t i = 0; i < bits.size(); ++i) {
        auto b = reader.read_bit();
        ASSERT_TRUE(b.has_value());
        ASSERT_EQ(*b, bits[i]);
    }
}

TEST_CASE("BitIO: flush with no data returns 0 padding") {
    std::ostringstream out;
    puff::BitWriter writer(out);
    uint8_t padding = writer.flush();
    ASSERT_EQ(static_cast<int>(padding), 0);
    ASSERT_EQ(writer.bytes_written(), static_cast<uint64_t>(0));
}

TEST_CASE("BitIO: bytes_written tracks correctly") {
    std::ostringstream out;
    puff::BitWriter writer(out);

    // Write 16 bits = 2 bytes.
    for (int i = 0; i < 16; ++i)
        writer.write_bit(i % 2 == 0);

    ASSERT_EQ(writer.bytes_written(), static_cast<uint64_t>(2));

    // Write 3 more bits, flush.
    writer.write_bit(true);
    writer.write_bit(false);
    writer.write_bit(true);
    writer.flush();

    ASSERT_EQ(writer.bytes_written(), static_cast<uint64_t>(3));
}

TEST_CASE("BitReader: read from empty stream") {
    std::istringstream in("");
    puff::BitReader reader(in);
    auto b = reader.read_bit();
    ASSERT_FALSE(b.has_value());
}
