#ifndef PUFF_ARCHIVE_ARCHIVE_READER_HPP
#define PUFF_ARCHIVE_ARCHIVE_READER_HPP

#include <filesystem>
#include <string>
#include <vector>

#include "puff/utils/types.hpp"

namespace puff {

/// Reads and extracts .puff archives.
class ArchiveReader {
public:
    /// Open an archive and parse its header, metadata, and file table.
    /// @throws std::runtime_error on invalid/corrupted archives.
    void open(const std::filesystem::path& archive_path);

    /// List all entries (files and directories) stored in the archive.
    [[nodiscard]] const std::vector<FileEntry>& list_entries() const noexcept;

    /// Get the parsed archive metadata.
    [[nodiscard]] const ArchiveMetadata& get_metadata() const noexcept;

    /// Get the total compressed payload size (excluding header/table/footer).
    [[nodiscard]] uint64_t compressed_payload_size() const noexcept;

    /// Extract all files to the given output directory.
    /// @throws std::runtime_error on decoding or I/O failures.
    void extract(const std::filesystem::path& output_dir) const;

private:
    std::filesystem::path archive_path_;
    ArchiveMetadata       metadata_;
    std::streamoff        payload_start_ = 0;   ///< Byte offset where payload begins.
    uint64_t              payload_size_  = 0;    ///< Total payload size.
    uint32_t              stored_crc_    = 0;    ///< CRC-32 from footer.

    // ── Binary deserialization helpers (little-endian) ───────────────────
    static uint8_t  read_u8 (std::istream& in);
    static uint16_t read_u16(std::istream& in);
    static uint32_t read_u32(std::istream& in);
    static uint64_t read_u64(std::istream& in);
};

}  // namespace puff

#endif  // PUFF_ARCHIVE_ARCHIVE_READER_HPP
