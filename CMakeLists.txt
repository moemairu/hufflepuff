cmake_minimum_required(VERSION 3.16)
project(hufflepuff
    VERSION 1.0.0
    DESCRIPTION "Cross-platform archive & compression utility powered by Huffman Coding"
    LANGUAGES CXX
)

# ── C++ Standard ─────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Guard against in-source builds ───────────────────────────────────────────
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not allowed. "
        "Please use: cmake -B build"
    )
endif()

# ── Compiler warnings ───────────────────────────────────────────────────────
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ── Subdirectories ───────────────────────────────────────────────────────────
add_subdirectory(src)

option(PUFF_BUILD_TESTS "Build the Hufflepuff test suite" ON)
if(PUFF_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
