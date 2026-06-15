#include "test_harness.hpp"

#include "puff/archive/archive_reader.hpp"
#include "puff/archive/archive_writer.hpp"
#include <filesystem>
#include <fstream>
#include <random>
#include <string>

namespace fs = std::filesystem;

static fs::path make_temp_dir() {
    auto temp = fs::temp_directory_path() / "puff_test_roundtrip";
    fs::create_directories(temp);
    return temp;
}

static void create_random_file(const fs::path& p, size_t size, unsigned int seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 255);
    std::string data;
    data.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        data.push_back(static_cast<char>(dist(rng)));
    }
    std::ofstream out(p, std::ios::binary);
    out.write(data.data(), data.size());
}

static bool files_are_identical(const fs::path& p1, const fs::path& p2) {
    std::ifstream f1(p1, std::ios::binary | std::ios::ate);
    std::ifstream f2(p2, std::ios::binary | std::ios::ate);

    if (f1.tellg() != f2.tellg()) return false;

    f1.seekg(0);
    f2.seekg(0);

    std::istreambuf_iterator<char> it1(f1);
    std::istreambuf_iterator<char> it2(f2);
    std::istreambuf_iterator<char> end;

    while (it1 != end && it2 != end) {
        if (*it1 != *it2) return false;
        ++it1;
        ++it2;
    }
    return true;
}

TEST_CASE("RoundTrip: Complex directory structure with binary data") {
    fs::path temp = make_temp_dir();
    fs::path src = temp / "source";
    fs::path ext = temp / "extracted";
    fs::path archive = temp / "archive.puff";

    fs::create_directories(src / "empty_dir");
    fs::create_directories(src / "data" / "bin");

    // Empty file
    std::ofstream(src / "empty_file.txt").put('\0');
    fs::resize_file(src / "empty_file.txt", 0); // ensure truly empty

    // Text file
    std::ofstream(src / "text.txt") << "Hello roundtrip!";

    // Binary file
    create_random_file(src / "data" / "bin" / "random1.dat", 1024, 123);
    create_random_file(src / "data" / "bin" / "random2.dat", 5000, 456);

    // Write archive
    puff::ArchiveWriter::create(src, archive);

    // Extract archive
    puff::ArchiveReader reader;
    reader.open(archive);
    reader.extract(ext);

    // Verify
    ASSERT_TRUE(fs::is_directory(ext / "empty_dir"));
    ASSERT_TRUE(fs::is_empty(ext / "empty_file.txt"));
    ASSERT_TRUE(files_are_identical(src / "text.txt", ext / "text.txt"));
    ASSERT_TRUE(files_are_identical(src / "data" / "bin" / "random1.dat", ext / "data" / "bin" / "random1.dat"));
    ASSERT_TRUE(files_are_identical(src / "data" / "bin" / "random2.dat", ext / "data" / "bin" / "random2.dat"));

    fs::remove_all(temp);
}
