#ifndef PUFF_UTILS_TYPES_HPP
#define PUFF_UTILS_TYPES_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace puff {

// ── Magic & version constants ───────────────────────────────────────────────
inline constexpr char     PUFF_MAGIC[4]       = {'P', 'U', 'F', 'F'};
inline constexpr uint8_t  PUFF_VERSION_MAJOR  = 0x01;
inline constexpr uint8_t  PUFF_VERSION_MINOR  = 0x00;

// ── Common type aliases ─────────────────────────────────────────────────────

/// Maps each byte value to its frequency count.
using ByteFrequencyMap = std::unordered_map<uint8_t, uint64_t>;

/// Maps each byte value to its Huffman code (sequence of bits).
using HuffmanCodeMap   = std::unordered_map<uint8_t, std::vector<bool>>;

// ── Result types ────────────────────────────────────────────────────────────

/// Returned by the encoder after compressing a stream.
struct EncodeResult {
    uint64_t compressed_size = 0;   ///< Size of compressed output in bytes.
    uint8_t  padding_bits    = 0;   ///< Number of zero-padding bits in the last byte (0–7).
};

/// Represents a single entry (file or directory) in a .puff archive.
struct FileEntry {
    enum class Type : uint8_t {
        File      = 0x00,
        Directory = 0x01,
    };

    Type        entry_type      = Type::File;
    std::string relative_path;                   ///< UTF-8, forward slashes, no leading "./"
    uint64_t    original_size   = 0;
    uint64_t    compressed_offset = 0;           ///< Byte offset into the payload section.
    uint64_t    compressed_size = 0;             ///< Compressed size (including freq table).
};

/// Top-level metadata stored in a .puff archive.
struct ArchiveMetadata {
    uint8_t  version_major    = PUFF_VERSION_MAJOR;
    uint8_t  version_minor    = PUFF_VERSION_MINOR;
    uint64_t creation_timestamp = 0;             ///< Unix epoch seconds.
    uint64_t original_total_size = 0;
    uint32_t file_count       = 0;
    uint32_t dir_count        = 0;
    std::vector<FileEntry> entries;
};

}  // namespace puff

#endif  // PUFF_UTILS_TYPES_HPP
