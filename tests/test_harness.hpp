/// @file test_harness.hpp
/// Minimal self-contained test harness — no external dependencies.

#ifndef PUFF_TEST_HARNESS_HPP
#define PUFF_TEST_HARNESS_HPP

#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace puff_test {

struct TestCase {
    std::string name;
    std::function<void()> func;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

inline int registrar(const std::string& name, std::function<void()> func) {
    registry().push_back({name, std::move(func)});
    return 0;
}

struct AssertionFailure : std::runtime_error {
    using std::runtime_error::runtime_error;
};

inline int run_all() {
    int passed = 0, failed = 0;

    for (const auto& tc : registry()) {
        try {
            tc.func();
            std::cout << "  ✓ " << tc.name << "\n";
            ++passed;
        } catch (const AssertionFailure& e) {
            std::cerr << "  ✗ " << tc.name << "\n"
                      << "    " << e.what() << "\n";
            ++failed;
        } catch (const std::exception& e) {
            std::cerr << "  ✗ " << tc.name << "  (exception)\n"
                      << "    " << e.what() << "\n";
            ++failed;
        }
    }

    std::cout << "\nResults: " << passed << " passed, "
              << failed << " failed, "
              << (passed + failed) << " total\n";

    return failed > 0 ? 1 : 0;
}

}  // namespace puff_test

// ── Macros ──────────────────────────────────────────────────────────────────

// Use a combination of a named function and __COUNTER__ to avoid collisions.
#define PUFF_TEST_CONCAT_INNER(a, b) a ## b
#define PUFF_TEST_CONCAT(a, b) PUFF_TEST_CONCAT_INNER(a, b)

#define TEST_CASE(name)                                                        \
    static void PUFF_TEST_CONCAT(_test_func_, __COUNTER__)();                  \
    namespace {                                                                \
        struct PUFF_TEST_CONCAT(_test_reg_, __COUNTER__) {                     \
            PUFF_TEST_CONCAT(_test_reg_, __COUNTER__)() {                      \
                puff_test::registrar(name,                                     \
                    PUFF_TEST_CONCAT(_test_func_, __COUNTER__ - 2));           \
            }                                                                  \
        } PUFF_TEST_CONCAT(_test_inst_, __COUNTER__);                          \
    }                                                                          \
    static void PUFF_TEST_CONCAT(_test_func_, __COUNTER__ - 4)()

// Simpler approach: use a global registration trick.
#undef TEST_CASE

// Let's use a simpler, more reliable pattern:
#define PUFF_TEST_UNIQUE_NAME(base) PUFF_TEST_CONCAT(base, __COUNTER__)

#define TEST_CASE(test_name)                                                   \
    static void PUFF_TEST_UNIQUE_NAME(_puff_test_fn_)();                       \
    static const int PUFF_TEST_UNIQUE_NAME(_puff_test_reg_) = [] {             \
        puff_test::registry().push_back(                                       \
            {test_name, PUFF_TEST_UNIQUE_NAME(_puff_test_fn_)});               \
        return 0;                                                              \
    }();                                                                       \
    static void PUFF_TEST_UNIQUE_NAME(_puff_test_fn_)()

// Hmm, the issue is __COUNTER__ increments each usage. Let me use the
// standard two-level approach with a local name capture.

#undef TEST_CASE

// Final reliable version: we use a static bool + lambda for registration.
#define TEST_CASE(test_name)                                                   \
    static void PUFF_TEST_CONCAT(puff_test_func_, __LINE__)();                 \
    static bool PUFF_TEST_CONCAT(puff_test_reg_, __LINE__) = [] {              \
        puff_test::registry().push_back(                                       \
            {test_name, PUFF_TEST_CONCAT(puff_test_func_, __LINE__)});         \
        return true;                                                           \
    }();                                                                       \
    static void PUFF_TEST_CONCAT(puff_test_func_, __LINE__)()

#define ASSERT_TRUE(expr)                                                      \
    do {                                                                       \
        if (!(expr)) {                                                         \
            std::ostringstream _ss;                                            \
            _ss << __FILE__ << ":" << __LINE__                                 \
                << ": ASSERT_TRUE(" #expr ") failed";                          \
            throw puff_test::AssertionFailure(_ss.str());                      \
        }                                                                      \
    } while (false)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

#define ASSERT_EQ(a, b)                                                        \
    do {                                                                       \
        const auto& _a = (a);                                                  \
        const auto& _b = (b);                                                  \
        if (_a != _b) {                                                        \
            std::ostringstream _ss;                                            \
            _ss << __FILE__ << ":" << __LINE__                                 \
                << ": ASSERT_EQ(" #a ", " #b ") failed: "                      \
                << _a << " != " << _b;                                         \
            throw puff_test::AssertionFailure(_ss.str());                      \
        }                                                                      \
    } while (false)

#define ASSERT_THROW(expr, exception_type)                                     \
    do {                                                                       \
        bool _caught = false;                                                  \
        try { expr; } catch (const exception_type&) { _caught = true; }        \
        if (!_caught) {                                                        \
            std::ostringstream _ss;                                            \
            _ss << __FILE__ << ":" << __LINE__                                 \
                << ": ASSERT_THROW(" #expr ", " #exception_type                \
                << ") — no exception thrown";                                   \
            throw puff_test::AssertionFailure(_ss.str());                      \
        }                                                                      \
    } while (false)

#endif  // PUFF_TEST_HARNESS_HPP
