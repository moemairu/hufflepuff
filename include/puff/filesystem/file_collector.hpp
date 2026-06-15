#ifndef PUFF_FILESYSTEM_FILE_COLLECTOR_HPP
#define PUFF_FILESYSTEM_FILE_COLLECTOR_HPP

#include <filesystem>
#include <vector>

namespace puff {

/// Result of collecting files and directories from a root path.
struct CollectionResult {
    std::vector<std::filesystem::path> files;   ///< Relative paths to files.
    std::vector<std::filesystem::path> dirs;    ///< Relative paths to directories.
};

/// Recursively collects files and directories under a given root.
class FileCollector {
public:
    /// Walk @p root recursively and return all files and (empty) directories
    /// as paths relative to @p root.
    ///
    /// Symlinks are skipped with a warning printed to stderr.
    /// Results are sorted lexicographically for deterministic output.
    static CollectionResult collect(const std::filesystem::path& root);
};

}  // namespace puff

#endif  // PUFF_FILESYSTEM_FILE_COLLECTOR_HPP
