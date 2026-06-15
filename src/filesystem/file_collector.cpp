#include "puff/filesystem/file_collector.hpp"

#include <algorithm>
#include <iostream>
#include <set>

namespace puff {

namespace fs = std::filesystem;

CollectionResult FileCollector::collect(const fs::path& root) {
    CollectionResult result;
    std::set<fs::path> dirs_with_children;

    for (auto it = fs::recursive_directory_iterator(
             root, fs::directory_options::skip_permission_denied);
         it != fs::recursive_directory_iterator(); ++it)
    {
        const auto& entry = *it;

        // Skip symlinks.
        if (entry.is_symlink()) {
            std::cerr << "Warning: skipping symlink: "
                      << entry.path().string() << "\n";
            continue;
        }

        fs::path relative = fs::relative(entry.path(), root);

        if (entry.is_regular_file()) {
            result.files.push_back(relative);

            // Mark parent directories as non-empty.
            if (relative.has_parent_path()) {
                fs::path parent = relative.parent_path();
                while (!parent.empty() && parent != ".") {
                    dirs_with_children.insert(parent);
                    parent = parent.parent_path();
                }
            }
        } else if (entry.is_directory()) {
            result.dirs.push_back(relative);
        }
    }

    // Keep only directories that are either empty or needed for structure.
    // Actually, we want ALL directories so extraction creates them,
    // but we especially want empty ones preserved.
    // The dirs vector already contains all directories.

    // Sort for deterministic output.
    std::sort(result.files.begin(), result.files.end());
    std::sort(result.dirs.begin(), result.dirs.end());

    return result;
}

}  // namespace puff
