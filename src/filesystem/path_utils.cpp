#include "puff/filesystem/path_utils.hpp"

#include <algorithm>
#include <stdexcept>

namespace puff {

namespace fs = std::filesystem;

std::string PathUtils::normalize_path(const fs::path& p) {
    std::string s = p.generic_string();  // Uses forward slashes.

    // Strip leading "./"
    if (s.size() >= 2 && s[0] == '.' && s[1] == '/') {
        s = s.substr(2);
    }

    return s;
}

fs::path PathUtils::make_output_path(const fs::path& input) {
    // For directories: strip trailing separator, then append .puff.
    // For files: replace extension with .puff.
    fs::path canonical_input = input;

    // Remove trailing slash from directories.
    std::string s = canonical_input.string();
    while (!s.empty() && (s.back() == '/' || s.back() == '\\')) {
        s.pop_back();
    }
    canonical_input = fs::path(s);

    if (fs::is_directory(input)) {
        return fs::path(canonical_input.string() + ".puff");
    }

    // For files: replace extension.
    fs::path result = canonical_input;
    result.replace_extension(".puff");
    return result;
}

fs::path PathUtils::sanitize_extract_path(const fs::path& relative,
                                          const fs::path& output_root) {
    // Check for directory traversal attempts.
    std::string rel_str = relative.generic_string();
    if (rel_str.find("..") != std::string::npos) {
        // More careful check: split into components.
        for (const auto& component : relative) {
            if (component == "..") {
                throw std::runtime_error(
                    "Path traversal attack detected in archive entry: " + rel_str);
            }
        }
    }

    fs::path result = output_root / relative;

    // Double-check that the resolved path is still under output_root.
    // We use lexically_relative to avoid filesystem access.
    fs::path rel_check = result.lexically_relative(output_root);
    std::string check_str = rel_check.generic_string();
    if (check_str.empty() || check_str.substr(0, 2) == "..") {
        throw std::runtime_error(
            "Path traversal attack detected in archive entry: " + rel_str);
    }

    return result;
}

}  // namespace puff
