#include "simple_test_lib.hpp"

int main(int argc, char *argv[]) {
    if (argc == 1) {
        test_core::run_all_tests();
    } else {
        for (int i = 1; i < argc; ++i) {
            if (not test_core::run_test(argv[i])) {
                std::cerr << "Invalid test name " 
                          << argv[i] << std::endl; 
            }
        }
    }
    cout_wrapper out;
    if (test_core::instance().all_success) {
        out << "ALL TESTS ARE SUCCESS" << std::endl;
    }
    return !test_core::instance().all_success;
}
