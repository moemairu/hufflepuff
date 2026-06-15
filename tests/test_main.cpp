#include "test_harness.hpp"

int main() {
    std::cout << "Hufflepuff Test Suite\n"
              << "====================\n\n";
    return puff_test::run_all();
}
