#ifndef PUFF_ARCHIVE_ARCHIVE_WRITER_HPP
#define PUFF_ARCHIVE_ARCHIVE_WRITER_HPP

#include <filesystem>
#include <string>

namespace puff {

/// Writes a .puff archive from a single file or a directory tree.
///
/// Archive layout (sequential binary stream, little-endian):
///   1. Magic header  (4 bytes "PUFF")
///   2. Version       (2 bytes: major, minor)
///   3. Metadata      (timestamps, sizes, counts)
///   4. File table    (entry_type, path, sizes, offsets per entry)
///   5. Payload       (per-file: frequency table + compressed bitstream)
///   6. CRC-32 footer (4 bytes over the payload section)
class ArchiveWriter {
public:
    /// Create a .puff archive from the given path.
    ///
    /// @param input_path  A file or directory to compress.
    /// @param output_path The destination .puff file (auto-generated if empty).
    /// @throws std::runtime_error on I/O or encoding failures.
    static void create(const std::filesystem::path& input_path,
                       const std::filesystem::path& output_path = {});

private:
    /// Helper: compress a single file's bytes and return the blob
    /// (frequency table + compressed bitstream) that goes in the payload.
    struct CompressedBlob {
        std::string data;          ///< Serialized freq table + bitstream.
        uint64_t    original_size; ///< Original file size.
    };

    static CompressedBlob compress_file(const std::filesystem::path& file_path);

    // ── Binary serialization helpers (little-endian) ─────────────────────
    static void write_u8 (std::ostream& out, uint8_t  v);
    static void write_u16(std::ostream& out, uint16_t v);
    static void write_u32(std::ostream& out, uint32_t v);
    static void write_u64(std::ostream& out, uint64_t v);
    static void write_bytes(std::ostream& out, const char* data, size_t len);
};

}  // namespace puff

#endif  // PUFF_ARCHIVE_ARCHIVE_WRITER_HPP
