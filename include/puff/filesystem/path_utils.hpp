#ifndef PUFF_FILESYSTEM_PATH_UTILS_HPP
#define PUFF_FILESYSTEM_PATH_UTILS_HPP

#include <filesystem>
#include <string>

namespace puff {

/// Utility functions for cross-platform path handling.
class PathUtils {
public:
    /// Convert a path to a normalized string with forward slashes and no
    /// leading "./" prefix.
    [[nodiscard]] static std::string normalize_path(const std::filesystem::path& p);

    /// Generate the default output path for compression.
    /// For a file "foo.txt" → "foo.puff".
    /// For a directory "mydir/" → "mydir.puff".
    [[nodiscard]] static std::filesystem::path make_output_path(
        const std::filesystem::path& input);

    /// Sanitize a relative path for extraction: resolve it under @p output_root
    /// and reject any path that would escape the root via "..".
    /// @throws std::runtime_error if the path attempts directory traversal.
    [[nodiscard]] static std::filesystem::path sanitize_extract_path(
        const std::filesystem::path& relative,
        const std::filesystem::path& output_root);
};

}  // namespace puff

#endif  // PUFF_FILESYSTEM_PATH_UTILS_HPP
