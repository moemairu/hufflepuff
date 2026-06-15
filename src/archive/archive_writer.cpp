#include "puff/archive/archive_writer.hpp"

#include "puff/huffman/encoder.hpp"
#include "puff/huffman/huffman_tree.hpp"
#include "puff/filesystem/file_collector.hpp"
#include "puff/filesystem/path_utils.hpp"
#include "puff/utils/crc32.hpp"
#include "puff/utils/types.hpp"

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace puff {

namespace fs = std::filesystem;

// ── Little-endian binary helpers ────────────────────────────────────────────

void ArchiveWriter::write_u8(std::ostream& out, uint8_t v) {
    out.put(static_cast<char>(v));
}

void ArchiveWriter::write_u16(std::ostream& out, uint16_t v) {
    out.put(static_cast<char>(v & 0xFF));
    out.put(static_cast<char>((v >> 8) & 0xFF));
}

void ArchiveWriter::write_u32(std::ostream& out, uint32_t v) {
    for (int i = 0; i < 4; ++i) {
        out.put(static_cast<char>((v >> (8 * i)) & 0xFF));
    }
}

void ArchiveWriter::write_u64(std::ostream& out, uint64_t v) {
    for (int i = 0; i < 8; ++i) {
        out.put(static_cast<char>((v >> (8 * i)) & 0xFF));
    }
}

void ArchiveWriter::write_bytes(std::ostream& out, const char* data, size_t len) {
    out.write(data, static_cast<std::streamsize>(len));
}

// ── Compress a single file ─────────────────────────────────────────────────

ArchiveWriter::CompressedBlob
ArchiveWriter::compress_file(const fs::path& file_path) {
    CompressedBlob blob;

    // Read entire file.
    std::ifstream in(file_path, std::ios::binary);
    if (!in)
        throw std::runtime_error("Cannot open file: " + file_path.string());

    std::ostringstream raw_buf;
    raw_buf << in.rdbuf();
    std::string raw_data = raw_buf.str();
    blob.original_size = raw_data.size();

    // Handle empty files.
    if (raw_data.empty()) {
        // Write an empty frequency table: 0 unique symbols, 0 padding.
        std::ostringstream out;
        write_u16(out, 0);      // unique_symbols = 0
        write_u8(out, 0);       // padding_bits = 0
        blob.data = out.str();
        return blob;
    }

    // 1) Calculate frequencies.
    std::istringstream freq_stream(raw_data);
    ByteFrequencyMap frequencies = Encoder::calculate_frequencies(freq_stream);

    // 2) Build Huffman tree and generate codes.
    HuffmanTree tree;
    tree.build(frequencies);
    HuffmanCodeMap codes = tree.generate_codes();

    // 3) Encode into a temporary buffer.
    std::istringstream encode_stream(raw_data);
    std::ostringstream compressed_buf;
    EncodeResult result = Encoder::encode(encode_stream, compressed_buf, codes);

    // 4) Serialize: frequency table header + compressed bitstream.
    std::ostringstream out;

    // Frequency table.
    auto unique_count = static_cast<uint16_t>(frequencies.size());
    write_u16(out, unique_count);
    for (const auto& [sym, freq] : frequencies) {
        write_u8(out, sym);
        write_u64(out, freq);
    }
    write_u8(out, result.padding_bits);

    // Compressed data.
    std::string compressed_data = compressed_buf.str();
    write_bytes(out, compressed_data.data(), compressed_data.size());

    blob.data = out.str();
    return blob;
}

// ── Create archive ─────────────────────────────────────────────────────────

void ArchiveWriter::create(const fs::path& input_path,
                           const fs::path& output_path_arg) {
    if (!fs::exists(input_path)) {
        throw std::runtime_error("Path not found: " + input_path.string());
    }

    // Determine output path.
    fs::path output_path = output_path_arg;
    if (output_path.empty()) {
        output_path = PathUtils::make_output_path(input_path);
    }

    // Collect files and directories.
    bool is_directory = fs::is_directory(input_path);
    std::vector<FileEntry> entries;
    std::vector<fs::path> absolute_file_paths;

    if (is_directory) {
        auto collection = FileCollector::collect(input_path);

        // Directory entries.
        for (const auto& dir : collection.dirs) {
            FileEntry entry;
            entry.entry_type    = FileEntry::Type::Directory;
            entry.relative_path = PathUtils::normalize_path(dir);
            entries.push_back(std::move(entry));
        }

        // File entries (sizes filled later).
        for (const auto& file : collection.files) {
            FileEntry entry;
            entry.entry_type    = FileEntry::Type::File;
            entry.relative_path = PathUtils::normalize_path(file);
            entries.push_back(std::move(entry));
            absolute_file_paths.push_back(input_path / file);
        }
    } else {
        FileEntry entry;
        entry.entry_type    = FileEntry::Type::File;
        entry.relative_path = input_path.filename().string();
        entries.push_back(std::move(entry));
        absolute_file_paths.push_back(input_path);
    }

    // ── First pass: compress all files ──────────────────────────────────

    std::vector<CompressedBlob> blobs;
    blobs.reserve(absolute_file_paths.size());

    for (const auto& file_path : absolute_file_paths) {
        blobs.push_back(compress_file(file_path));
    }

    // ── Calculate offsets and sizes ─────────────────────────────────────

    uint64_t payload_offset = 0;
    size_t blob_idx = 0;
    uint64_t original_total = 0;
    uint32_t file_count = 0;
    uint32_t dir_count  = 0;

    for (auto& entry : entries) {
        if (entry.entry_type == FileEntry::Type::Directory) {
            ++dir_count;
            continue;
        }

        const auto& blob = blobs[blob_idx];
        entry.original_size     = blob.original_size;
        entry.compressed_offset = payload_offset;
        entry.compressed_size   = blob.data.size();

        payload_offset    += blob.data.size();
        original_total    += blob.original_size;
        ++file_count;
        ++blob_idx;
    }

    // ── Second pass: write the archive ──────────────────────────────────

    std::ofstream out(output_path, std::ios::binary);
    if (!out)
        throw std::runtime_error("Cannot create output file: " + output_path.string());

    // Section 1: Magic header.
    write_bytes(out, PUFF_MAGIC, 4);

    // Section 2: Version.
    write_u8(out, PUFF_VERSION_MAJOR);
    write_u8(out, PUFF_VERSION_MINOR);

    // Section 3: Archive metadata.
    auto now = std::chrono::system_clock::now();
    auto epoch = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()).count());

    write_u64(out, epoch);
    write_u64(out, original_total);
    write_u32(out, file_count);
    write_u32(out, dir_count);

    // Section 4: File table.
    for (const auto& entry : entries) {
        write_u8(out, static_cast<uint8_t>(entry.entry_type));

        auto path_len = static_cast<uint16_t>(entry.relative_path.size());
        write_u16(out, path_len);
        write_bytes(out, entry.relative_path.data(), path_len);

        write_u64(out, entry.original_size);
        write_u64(out, entry.compressed_offset);
        write_u64(out, entry.compressed_size);
    }

    // Section 5: Payload (concatenated compressed blobs).
    // Also compute CRC-32 over the payload.
    uint32_t crc = crc32_init();
    for (const auto& blob : blobs) {
        write_bytes(out, blob.data.data(), blob.data.size());
        crc = crc32_update(crc,
                           reinterpret_cast<const uint8_t*>(blob.data.data()),
                           blob.data.size());
    }
    crc = crc32_finalize(crc);

    // Section 6: CRC-32 footer.
    write_u32(out, crc);

    out.flush();
    if (!out)
        throw std::runtime_error("Failed to write archive: " + output_path.string());

    // Print summary.
    uint64_t compressed_total = 0;
    for (const auto& blob : blobs)
        compressed_total += blob.data.size();

    double ratio = original_total > 0
        ? (1.0 - static_cast<double>(compressed_total) /
                  static_cast<double>(original_total)) * 100.0
        : 0.0;

    std::cout << "Created: " << output_path.string() << "\n"
              << "  Files: " << file_count;
    if (dir_count > 0)
        std::cout << " (" << dir_count << " directories)";
    std::cout << "\n"
              << "  Original:   " << original_total << " bytes\n"
              << "  Compressed: " << compressed_total << " bytes\n"
              << "  Ratio:      " << std::fixed
              << std::setprecision(1) << ratio << "% reduction\n";
}

}  // namespace puff
