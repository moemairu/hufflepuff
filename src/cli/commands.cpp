#include "puff/cli/commands.hpp"

#include "puff/archive/archive_reader.hpp"
#include "puff/archive/archive_writer.hpp"
#include "puff/benchmark/benchmark_engine.hpp"
#include "puff/cli/command_parser.hpp"
#include "puff/utils/types.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace puff {

namespace fs = std::filesystem;

// ── compress ────────────────────────────────────────────────────────────────

int cmd_compress(const std::string& path) {
    try {
        ArchiveWriter::create(fs::path(path));
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

// ── extract ─────────────────────────────────────────────────────────────────

int cmd_extract(const std::string& archive) {
    try {
        ArchiveReader reader;
        reader.open(fs::path(archive));

        // Extract to current directory.
        // If the archive has multiple files, create a subdirectory named
        // after the archive (without .puff extension).
        fs::path output_dir = ".";
        const auto& meta = reader.get_metadata();

        if (meta.file_count > 1 || meta.dir_count > 0) {
            fs::path archive_path(archive);
            output_dir = archive_path.stem();
        }

        reader.extract(output_dir);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

// ── list ────────────────────────────────────────────────────────────────────

int cmd_list(const std::string& archive) {
    try {
        ArchiveReader reader;
        reader.open(fs::path(archive));

        const auto& entries = reader.list_entries();
        for (const auto& entry : entries) {
            if (entry.entry_type == FileEntry::Type::Directory) {
                std::cout << entry.relative_path << "/\n";
            } else {
                std::cout << entry.relative_path << "\n";
            }
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

// ── info ────────────────────────────────────────────────────────────────────

int cmd_info(const std::string& archive) {
    try {
        ArchiveReader reader;
        reader.open(fs::path(archive));

        const auto& meta = reader.get_metadata();
        uint64_t compressed_total = reader.compressed_payload_size();

        double ratio = meta.original_total_size > 0
            ? (1.0 - static_cast<double>(compressed_total) /
                      static_cast<double>(meta.original_total_size)) * 100.0
            : 0.0;

        // Format timestamp.
        auto time_point = std::chrono::system_clock::time_point(
            std::chrono::seconds(meta.creation_timestamp));
        auto time_t_val = std::chrono::system_clock::to_time_t(time_point);
        std::tm tm_val{};
#if defined(_WIN32)
        localtime_s(&tm_val, &time_t_val);
#else
        localtime_r(&time_t_val, &tm_val);
#endif
        std::ostringstream time_ss;
        time_ss << std::put_time(&tm_val, "%Y-%m-%d %H:%M:%S");

        std::cout << "Archive:      " << fs::path(archive).filename().string() << "\n"
                  << "Version:      v" << static_cast<int>(meta.version_major) << "."
                  << static_cast<int>(meta.version_minor) << "\n"
                  << "Created:      " << time_ss.str() << "\n"
                  << "Files:        " << meta.file_count << "\n"
                  << "Directories:  " << meta.dir_count << "\n"
                  << "Original:     " << meta.original_total_size << " bytes\n"
                  << "Compressed:   " << compressed_total << " bytes\n"
                  << "Ratio:        " << std::fixed << std::setprecision(1)
                  << ratio << "% reduction\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

// ── benchmark ───────────────────────────────────────────────────────────────

int cmd_benchmark(const std::string& file) {
    try {
        BenchmarkEngine engine;
        auto result = engine.run(fs::path(file));
        engine.print_report(result);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

// ── help ────────────────────────────────────────────────────────────────────

int cmd_help(const std::string& program_name) {
    CommandParser::print_usage(program_name);
    return 0;
}

}  // namespace puff
