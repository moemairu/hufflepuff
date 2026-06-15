#include "test_harness.hpp"

#include "puff/archive/archive_reader.hpp"
#include "puff/archive/archive_writer.hpp"
#include "puff/archive/archive_validator.hpp"
#include "puff/utils/crc32.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

// ── Helpers ─────────────────────────────────────────────────────────────────

static fs::path make_temp_dir() {
    auto temp = fs::temp_directory_path() / "puff_test_archive";
    fs::create_directories(temp);
    return temp;
}

static void create_file(const fs::path& p, const std::string& content) {
    std::ofstream out(p, std::ios::binary);
    out << content;
}

static std::string read_file(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// ── Tests ───────────────────────────────────────────────────────────────────

TEST_CASE("Archive: write and read single file archive") {
    fs::path temp_dir = make_temp_dir();
    fs::path input_file = temp_dir / "single.txt";
    fs::path archive_file = temp_dir / "single.puff";
    fs::path extract_dir = temp_dir / "extract_single";

    std::string content = "This is a single file test.";
    create_file(input_file, content);

    // Write
    puff::ArchiveWriter::create(input_file, archive_file);
    ASSERT_TRUE(fs::exists(archive_file));

    // Read & verify info
    puff::ArchiveReader reader;
    reader.open(archive_file);

    const auto& meta = reader.get_metadata();
    ASSERT_EQ(meta.file_count, static_cast<uint32_t>(1));
    ASSERT_EQ(meta.dir_count, static_cast<uint32_t>(0));
    ASSERT_EQ(meta.original_total_size, static_cast<uint64_t>(content.size()));

    const auto& entries = reader.list_entries();
    ASSERT_EQ(entries.size(), static_cast<size_t>(1));
    ASSERT_EQ(entries[0].relative_path, "single.txt");
    ASSERT_EQ(entries[0].original_size, static_cast<uint64_t>(content.size()));

    // Extract
    reader.extract(extract_dir);

    // Verify
    std::string extracted_content = read_file(extract_dir / "single.txt");
    ASSERT_EQ(extracted_content, content);

    fs::remove_all(temp_dir);
}

TEST_CASE("ArchiveValidator: CRC32 correctness") {
    std::string data = "123456789";
    uint32_t expected = 0xCBF43926; // Standard test vector
    uint32_t actual = puff::crc32(reinterpret_cast<const uint8_t*>(data.data()), data.size());
    ASSERT_EQ(actual, expected);
}

TEST_CASE("ArchiveValidator: detect invalid magic bytes") {
    fs::path temp_dir = make_temp_dir();
    fs::path bad_archive = temp_dir / "bad_magic.puff";

    create_file(bad_archive, "BADFv1...");

    puff::ArchiveReader reader;
    ASSERT_THROW(reader.open(bad_archive), std::runtime_error);

    fs::remove_all(temp_dir);
}

TEST_CASE("ArchiveValidator: detect unsupported version") {
    fs::path temp_dir = make_temp_dir();
    fs::path bad_archive = temp_dir / "bad_version.puff";

    std::string content = "PUFF";
    content.push_back(0x02); // Major version 2 (we only support 1)
    content.push_back(0x00); // Minor version 0
    create_file(bad_archive, content);

    puff::ArchiveReader reader;
    ASSERT_THROW(reader.open(bad_archive), std::runtime_error);

    fs::remove_all(temp_dir);
}

TEST_CASE("Archive: write and read multi-file archive with directory") {
    fs::path temp_dir = make_temp_dir();
    fs::path source_dir = temp_dir / "src_dir";
    fs::path archive_file = temp_dir / "multi.puff";
    fs::path extract_dir = temp_dir / "extract_multi";

    fs::create_directories(source_dir / "folder1");
    fs::create_directories(source_dir / "folder2"); // Empty dir

    create_file(source_dir / "file1.txt", "File 1 content");
    create_file(source_dir / "folder1" / "file2.txt", "File 2 content inside folder");

    // Write
    puff::ArchiveWriter::create(source_dir, archive_file);

    // Read
    puff::ArchiveReader reader;
    reader.open(archive_file);

    const auto& entries = reader.list_entries();
    // Expected entries: file1.txt, folder1, folder1/file2.txt, folder2
    // Order might be sorted. Let's just check the counts.
    ASSERT_EQ(reader.get_metadata().file_count, static_cast<uint32_t>(2));
    ASSERT_EQ(reader.get_metadata().dir_count, static_cast<uint32_t>(2));
    ASSERT_EQ(entries.size(), static_cast<size_t>(4));

    // Extract
    reader.extract(extract_dir);

    // Verify
    ASSERT_EQ(read_file(extract_dir / "file1.txt"), "File 1 content");
    ASSERT_EQ(read_file(extract_dir / "folder1" / "file2.txt"), "File 2 content inside folder");
    ASSERT_TRUE(fs::is_directory(extract_dir / "folder2"));

    fs::remove_all(temp_dir);
}
