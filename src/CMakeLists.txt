# ── Library: puff_lib ─────────────────────────────────────────────────────────
# All source modules compiled into a static library so that both the
# executable and the test binary can link against them.

set(PUFF_LIB_SOURCES
    # Huffman engine
    huffman/huffman_tree.cpp
    huffman/encoder.cpp
    huffman/decoder.cpp
    huffman/bit_writer.cpp
    huffman/bit_reader.cpp

    # Archive format
    archive/archive_writer.cpp
    archive/archive_reader.cpp
    archive/archive_validator.cpp

    # Filesystem utilities
    filesystem/file_collector.cpp
    filesystem/path_utils.cpp

    # CLI
    cli/command_parser.cpp
    cli/commands.cpp

    # Benchmark
    benchmark/benchmark_engine.cpp

    # Utilities
    utils/crc32.cpp
)

add_library(puff_lib STATIC ${PUFF_LIB_SOURCES})

target_include_directories(puff_lib
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(puff_lib PUBLIC cxx_std_20)

# ── Executable: puff ─────────────────────────────────────────────────────────

add_executable(puff main.cpp)
target_link_libraries(puff PRIVATE puff_lib)
