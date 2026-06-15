#ifndef PUFF_BENCHMARK_BENCHMARK_ENGINE_HPP
#define PUFF_BENCHMARK_BENCHMARK_ENGINE_HPP

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace puff {

/// Results of a compression benchmark.
struct BenchmarkResult {
    std::string file_name;

    uint64_t original_size    = 0;
    uint64_t compressed_size  = 0;   ///< Bitstream only (no freq table overhead).

    double compression_ratio  = 0.0; ///< (1 - compressed/original) * 100
    double encoding_time_ms   = 0.0;
    double decoding_time_ms   = 0.0;
    double avg_code_length    = 0.0;
    double shannon_entropy    = 0.0; ///< Bits per symbol.
    double efficiency_pct     = 0.0; ///< (entropy / avg_code_length) * 100

    /// Top N most frequent symbols.
    struct SymbolFreq {
        uint8_t  symbol;
        uint64_t frequency;
        double   percentage;      ///< frequency / total * 100
    };
    std::vector<SymbolFreq> top_symbols;
};

/// Runs a compression benchmark on a single file.
class BenchmarkEngine {
public:
    /// Run the benchmark and return the results.
    BenchmarkResult run(const std::filesystem::path& file_path);

    /// Print a formatted report to stdout.
    static void print_report(const BenchmarkResult& result);
};

}  // namespace puff

#endif  // PUFF_BENCHMARK_BENCHMARK_ENGINE_HPP
