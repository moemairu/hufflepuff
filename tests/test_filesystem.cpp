#include "test_harness.hpp"

#include "puff/filesystem/file_collector.hpp"
#include "puff/filesystem/path_utils.hpp"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

static fs::path make_temp_dir() {
    auto temp = fs::temp_directory_path() / "puff_test_fs";
    fs::create_directories(temp);
    return temp;
}

TEST_CASE("PathUtils: normalize_path") {
    // Basic normalization
#ifdef _WIN32
    // generic_string on Windows converts \ to /
    ASSERT_EQ(puff::PathUtils::normalize_path(fs::path("a\\b\\c")), "a/b/c");
#endif
    ASSERT_EQ(puff::PathUtils::normalize_path(fs::path("a/b/c")), "a/b/c");

    // Strip leading ./
    ASSERT_EQ(puff::PathUtils::normalize_path(fs::path("./a/b")), "a/b");
}

TEST_CASE("PathUtils: make_output_path") {
    fs::path p1("test.txt");
    ASSERT_EQ(puff::PathUtils::make_output_path(p1), fs::path("test.puff"));

    fs::path p2("folder/subfolder"); // We pretend this is a dir using the actual filesystem or checking it directly inside make_output_path?
    // Note: make_output_path checks fs::is_directory. So we need a real directory to test directory logic.
    fs::path temp = make_temp_dir();
    fs::path d = temp / "my_dir";
    fs::create_directories(d);

    // Dir logic
    ASSERT_EQ(puff::PathUtils::make_output_path(d), fs::path(d.string() + ".puff"));

    // Trailing slash
    fs::path d_slash(d.string() + "/");
    ASSERT_EQ(puff::PathUtils::make_output_path(d_slash), fs::path(d.string() + ".puff"));

    fs::remove_all(temp);
}

TEST_CASE("PathUtils: sanitize_extract_path prevents traversal") {
    fs::path root = "/extract_root";

    // Safe paths
    ASSERT_EQ(puff::PathUtils::sanitize_extract_path("safe.txt", root), root / "safe.txt");
    ASSERT_EQ(puff::PathUtils::sanitize_extract_path("dir/safe.txt", root), root / "dir/safe.txt");

    // Traversal attempts
    ASSERT_THROW(puff::PathUtils::sanitize_extract_path("../escaped.txt", root), std::runtime_error);
    ASSERT_THROW(puff::PathUtils::sanitize_extract_path("dir/../../escaped.txt", root), std::runtime_error);
}

TEST_CASE("FileCollector: collect files and directories") {
    fs::path temp = make_temp_dir();
    fs::create_directories(temp / "dir1");
    fs::create_directories(temp / "dir2" / "dir3");

    std::ofstream(temp / "file1.txt") << "data";
    std::ofstream(temp / "dir1" / "file2.txt") << "data";

    auto result = puff::FileCollector::collect(temp);

    // Expected files: file1.txt, dir1/file2.txt
    ASSERT_EQ(result.files.size(), static_cast<size_t>(2));

    // Expected dirs: dir1, dir2, dir2/dir3
    // Note: the collector finds all dirs
    ASSERT_EQ(result.dirs.size(), static_cast<size_t>(3));

    fs::remove_all(temp);
}
