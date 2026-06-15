#include "puff/archive/archive_reader.hpp"

#include "puff/archive/archive_validator.hpp"
#include "puff/huffman/decoder.hpp"
#include "puff/huffman/encoder.hpp"
#include "puff/huffman/huffman_tree.hpp"
#include "puff/filesystem/path_utils.hpp"
#include "puff/utils/crc32.hpp"
#include "puff/utils/types.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace puff {

namespace fs = std::filesystem;

// ── Little-endian read helpers ──────────────────────────────────────────────

uint8_t ArchiveReader::read_u8(std::istream& in) {
    char ch;
    if (!in.get(ch))
        throw std::runtime_error("Unexpected end of archive");
    return static_cast<uint8_t>(ch);
}

uint16_t ArchiveReader::read_u16(std::istream& in) {
    uint16_t v = 0;
    for (int i = 0; i < 2; ++i)
        v |= static_cast<uint16_t>(read_u8(in)) << (8 * i);
    return v;
}

uint32_t ArchiveReader::read_u32(std::istream& in) {
    uint32_t v = 0;
    for (int i = 0; i < 4; ++i)
        v |= static_cast<uint32_t>(read_u8(in)) << (8 * i);
    return v;
}

uint64_t ArchiveReader::read_u64(std::istream& in) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i)
        v |= static_cast<uint64_t>(read_u8(in)) << (8 * i);
    return v;
}

// ── Open & parse ────────────────────────────────────────────────────────────

void ArchiveReader::open(const fs::path& archive_path) {
    archive_path_ = archive_path;

    std::ifstream in(archive_path_, std::ios::binary);
    if (!in)
        throw std::runtime_error("Cannot open archive: " + archive_path_.string());

    // Validate magic.
    if (!ArchiveValidator::validate_magic(in))
        throw std::runtime_error(ArchiveValidator::last_error());

    // Consume magic.
    in.seekg(4);

    // Read version.
    uint8_t major = read_u8(in);
    uint8_t minor = read_u8(in);
    if (!ArchiveValidator::validate_version(major, minor))
        throw std::runtime_error(ArchiveValidator::last_error());

    metadata_.version_major = major;
    metadata_.version_minor = minor;

    // Read metadata.
    metadata_.creation_timestamp = read_u64(in);
    metadata_.original_total_size = read_u64(in);
    metadata_.file_count = read_u32(in);
    metadata_.dir_count  = read_u32(in);

    // Read file table.
    uint32_t total_entries = metadata_.file_count + metadata_.dir_count;
    metadata_.entries.clear();
    metadata_.entries.reserve(total_entries);

    for (uint32_t i = 0; i < total_entries; ++i) {
        FileEntry entry;
        entry.entry_type = static_cast<FileEntry::Type>(read_u8(in));

        uint16_t path_len = read_u16(in);
        entry.relative_path.resize(path_len);
        in.read(entry.relative_path.data(), path_len);

        entry.original_size     = read_u64(in);
        entry.compressed_offset = read_u64(in);
        entry.compressed_size   = read_u64(in);

        metadata_.entries.push_back(std::move(entry));
    }

    // Record where the payload starts.
    payload_start_ = in.tellg();

    // Calculate total payload size.
    payload_size_ = 0;
    for (const auto& entry : metadata_.entries) {
        if (entry.entry_type == FileEntry::Type::File) {
            payload_size_ += entry.compressed_size;
        }
    }

    // Read CRC footer: it's at the end of the file, after the payload.
    in.seekg(payload_start_ + static_cast<std::streamoff>(payload_size_));
    stored_crc_ = read_u32(in);

    // Verify CRC.
    in.seekg(payload_start_);
    std::vector<uint8_t> payload_data(payload_size_);
    if (payload_size_ > 0) {
        in.read(reinterpret_cast<char*>(payload_data.data()),
                static_cast<std::streamsize>(payload_size_));
    }

    if (!ArchiveValidator::validate_crc(payload_data.data(),
                                        payload_data.size(),
                                        stored_crc_)) {
        throw std::runtime_error(ArchiveValidator::last_error());
    }
}

// ── Accessors ───────────────────────────────────────────────────────────────

const std::vector<FileEntry>& ArchiveReader::list_entries() const noexcept {
    return metadata_.entries;
}

const ArchiveMetadata& ArchiveReader::get_metadata() const noexcept {
    return metadata_;
}

uint64_t ArchiveReader::compressed_payload_size() const noexcept {
    return payload_size_;
}

// ── Extract ─────────────────────────────────────────────────────────────────

void ArchiveReader::extract(const fs::path& output_dir) const {
    // Create output directory if needed.
    fs::create_directories(output_dir);

    std::ifstream in(archive_path_, std::ios::binary);
    if (!in)
        throw std::runtime_error("Cannot open archive: " + archive_path_.string());

    for (const auto& entry : metadata_.entries) {
        fs::path target = PathUtils::sanitize_extract_path(
            fs::path(entry.relative_path), output_dir);

        if (entry.entry_type == FileEntry::Type::Directory) {
            fs::create_directories(target);
            continue;
        }

        // Ensure parent directory exists.
        if (target.has_parent_path())
            fs::create_directories(target.parent_path());

        // Handle empty files.
        if (entry.original_size == 0) {
            std::ofstream out(target, std::ios::binary);
            if (!out)
                throw std::runtime_error("Cannot create file: " + target.string());
            continue;
        }

        // Seek to this file's data in the payload.
        in.seekg(payload_start_ + static_cast<std::streamoff>(entry.compressed_offset));

        // Read frequency table.
        uint16_t unique_symbols = read_u16(in);
        ByteFrequencyMap frequencies;
        for (uint16_t s = 0; s < unique_symbols; ++s) {
            uint8_t sym   = read_u8(in);
            uint64_t freq = read_u64(in);
            frequencies[sym] = freq;
        }
        uint8_t padding_bits = read_u8(in);

        // Rebuild Huffman tree.
        HuffmanTree tree;
        tree.build(frequencies);

        // Calculate the compressed bitstream size.
        // compressed_size = freq_table_size + bitstream_size
        // freq_table_size = 2 (unique_symbols) + unique_symbols*(1+8) + 1 (padding)
        uint64_t freq_table_size = 2 + static_cast<uint64_t>(unique_symbols) * 9 + 1;
        uint64_t bitstream_size  = entry.compressed_size - freq_table_size;

        // Read the compressed bitstream into memory so we can decode it.
        std::string bitstream_data(bitstream_size, '\0');
        in.read(bitstream_data.data(), static_cast<std::streamsize>(bitstream_size));

        std::istringstream bitstream_in(bitstream_data);
        std::ofstream out(target, std::ios::binary);
        if (!out)
            throw std::runtime_error("Cannot create file: " + target.string());

        Decoder::decode(bitstream_in, out, tree, entry.original_size, padding_bits);
    }

    std::cout << "Extracted to: " << output_dir.string() << "\n"
              << "  Files: " << metadata_.file_count;
    if (metadata_.dir_count > 0)
        std::cout << " (" << metadata_.dir_count << " directories)";
    std::cout << "\n";
}

}  // namespace puff
