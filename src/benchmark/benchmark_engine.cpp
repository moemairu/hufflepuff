#include "puff/benchmark/benchmark_engine.hpp"

#include "puff/huffman/decoder.hpp"
#include "puff/huffman/encoder.hpp"
#include "puff/huffman/huffman_tree.hpp"
#include "puff/utils/types.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace puff {

namespace fs = std::filesystem;

BenchmarkResult BenchmarkEngine::run(const fs::path& file_path) {
    BenchmarkResult result;
    result.file_name = file_path.filename().string();

    // Read entire file into memory.
    std::ifstream in(file_path, std::ios::binary);
    if (!in)
        throw std::runtime_error("Cannot open file: " + file_path.string());

    std::ostringstream buf;
    buf << in.rdbuf();
    std::string raw_data = buf.str();
    result.original_size = raw_data.size();

    if (raw_data.empty())
        throw std::runtime_error("File is empty, nothing to benchmark");

    // Calculate frequencies.
    std::istringstream freq_stream(raw_data);
    ByteFrequencyMap frequencies = Encoder::calculate_frequencies(freq_stream);

    // Build tree and generate codes.
    HuffmanTree tree;
    tree.build(frequencies);
    HuffmanCodeMap codes = tree.generate_codes();

    // ── Encoding benchmark ──────────────────────────────────────────────

    std::string compressed_data;
    EncodeResult encode_result;

    {
        std::istringstream encode_in(raw_data);
        std::ostringstream encode_out;

        auto t0 = std::chrono::high_resolution_clock::now();
        encode_result = Encoder::encode(encode_in, encode_out, codes);
        auto t1 = std::chrono::high_resolution_clock::now();

        result.encoding_time_ms =
            std::chrono::duration<double, std::milli>(t1 - t0).count();
        compressed_data = encode_out.str();
    }

    result.compressed_size = encode_result.compressed_size;

    // ── Decoding benchmark ──────────────────────────────────────────────

    {
        std::istringstream decode_in(compressed_data);
        std::ostringstream decode_out;

        auto t0 = std::chrono::high_resolution_clock::now();
        Decoder::decode(decode_in, decode_out, tree,
                        result.original_size, encode_result.padding_bits);
        auto t1 = std::chrono::high_resolution_clock::now();

        result.decoding_time_ms =
            std::chrono::duration<double, std::milli>(t1 - t0).count();
    }

    // ── Statistics ──────────────────────────────────────────────────────

    result.compression_ratio = result.original_size > 0
        ? (1.0 - static_cast<double>(result.compressed_size) /
                  static_cast<double>(result.original_size)) * 100.0
        : 0.0;

    // Average code length (weighted by frequency).
    double total_bits = 0;
    uint64_t total_symbols = 0;
    for (const auto& [sym, freq] : frequencies) {
        auto it = codes.find(sym);
        if (it != codes.end()) {
            total_bits   += static_cast<double>(freq) *
                            static_cast<double>(it->second.size());
            total_symbols += freq;
        }
    }
    result.avg_code_length = total_symbols > 0
        ? total_bits / static_cast<double>(total_symbols)
        : 0.0;

    // Shannon entropy: H = -Σ p(x) log₂(p(x))
    double entropy = 0.0;
    for (const auto& [sym, freq] : frequencies) {
        double p = static_cast<double>(freq) / static_cast<double>(total_symbols);
        if (p > 0)
            entropy -= p * std::log2(p);
    }
    result.shannon_entropy = entropy;

    // Compression efficiency.
    result.efficiency_pct = result.avg_code_length > 0
        ? (result.shannon_entropy / result.avg_code_length) * 100.0
        : 0.0;

    // Top 10 most frequent symbols.
    std::vector<std::pair<uint8_t, uint64_t>> freq_vec(
        frequencies.begin(), frequencies.end());
    std::sort(freq_vec.begin(), freq_vec.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });

    size_t top_n = std::min<size_t>(10, freq_vec.size());
    result.top_symbols.reserve(top_n);
    for (size_t i = 0; i < top_n; ++i) {
        BenchmarkResult::SymbolFreq sf;
        sf.symbol     = freq_vec[i].first;
        sf.frequency  = freq_vec[i].second;
        sf.percentage = static_cast<double>(sf.frequency) /
                        static_cast<double>(total_symbols) * 100.0;
        result.top_symbols.push_back(sf);
    }

    return result;
}

void BenchmarkEngine::print_report(const BenchmarkResult& result) {
    auto format_symbol = [](uint8_t sym) -> std::string {
        if (sym >= 32 && sym < 127) {
            return std::string("'") + static_cast<char>(sym) + "'";
        }
        char buf[8];
        std::snprintf(buf, sizeof(buf), "0x%02X", sym);
        return std::string(buf);
    };

    std::cout << "\n"
              << "╔══════════════════════════════════════════════╗\n"
              << "║         Hufflepuff Benchmark Report          ║\n"
              << "╠══════════════════════════════════════════════╣\n"
              << "║  File: " << std::left << std::setw(37) << result.file_name << " ║\n"
              << "╠══════════════════════════════════════════════╣\n";

    std::cout << std::fixed << std::setprecision(2);

    std::cout << "║  Original Size:      " << std::setw(12) << result.original_size
              << " bytes      ║\n"
              << "║  Compressed Size:    " << std::setw(12) << result.compressed_size
              << " bytes      ║\n"
              << "║  Compression Ratio:  " << std::setw(12) << result.compression_ratio
              << " %          ║\n"
              << "╠══════════════════════════════════════════════╣\n"
              << "║  Encoding Time:      " << std::setw(12) << result.encoding_time_ms
              << " ms         ║\n"
              << "║  Decoding Time:      " << std::setw(12) << result.decoding_time_ms
              << " ms         ║\n"
              << "╠══════════════════════════════════════════════╣\n"
              << "║  Avg Code Length:    " << std::setw(12) << result.avg_code_length
              << " bits       ║\n"
              << "║  Shannon Entropy:    " << std::setw(12) << result.shannon_entropy
              << " bits       ║\n"
              << "║  Efficiency:         " << std::setw(12) << result.efficiency_pct
              << " %          ║\n"
              << "╠══════════════════════════════════════════════╣\n"
              << "║  Top Symbols:                                ║\n";

    for (const auto& sf : result.top_symbols) {
        std::ostringstream line;
        line << "    " << std::left << std::setw(6) << format_symbol(sf.symbol)
             << "  " << std::setw(10) << sf.frequency
             << "  (" << std::fixed << std::setprecision(1)
             << sf.percentage << "%)";
        std::cout << "║  " << std::left << std::setw(43) << line.str() << " ║\n";
    }

    std::cout << "╚══════════════════════════════════════════════╝\n\n";
}

}  // namespace puff
