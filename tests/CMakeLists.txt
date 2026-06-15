# ── Test executable: puff_tests ──────────────────────────────────────────────

set(PUFF_TEST_SOURCES
    test_main.cpp
    test_huffman_tree.cpp
    test_bit_io.cpp
    test_encoder_decoder.cpp
    test_archive.cpp
    test_filesystem.cpp
    test_roundtrip.cpp
)

add_executable(puff_tests ${PUFF_TEST_SOURCES})

# Link against the main library
target_link_libraries(puff_tests PRIVATE puff_lib)

# Register tests with CTest
add_test(NAME puff_tests COMMAND puff_tests)
