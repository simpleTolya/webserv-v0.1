#ifndef FT_SIMPLE_TEST_LIB_HPP
# define FT_SIMPLE_TEST_LIB_HPP

////////////////////////////////////////////////
//
//  Very simple testing library done in an hour
//
////////////////////////////////////////////////

# include <string>
# include <functional>
# include <utility>
# include <iostream>
# include <atomic>
# include <unordered_map>
# include <sstream>
# include <atomic>
# include <mutex>
# include <chrono>

struct i_test_case {
    virtual ~i_test_case() {}
    virtual void run() = 0;
    virtual const std::string & name() = 0;
};

struct cout_wrapper : public std::ostringstream {
private:
    static std::recursive_mutex & get_lock() {
        static std::recursive_mutex lock_cout;
        return lock_cout;
    }
public:
    ~cout_wrapper() {
        std::lock_guard lock(get_lock());
        std::cout << this->str();
    }
};


struct test_core {
private:
    std::unordered_map<std::string, i_test_case*> tests;

public:
    std::atomic_bool all_success = true;

    static void register_assert_true(bool value, const char *expr, int line) {
        cout_wrapper out;
        if (not value) {
            instance().all_success = false;
        }
        if (value) {
            out << "SUCCESS: " << expr << " IS TRUE";
        } else {
            out << "FAIL in ASSERT_TRUE: " << expr << " IS FALSE"
                      << " (in line " << line << ")";
        }
        out << std::endl;
    }

    static void register_exception_handling(bool handled, int line) {
        cout_wrapper out;
        if (not handled) {
            instance().all_success = false;
        }
        if (handled) {
            out << "SUCCESS: Correct exception handled";
        } else {
            out << "FAIL in ASSERT_EXCEPTION: " << 
                "NOT HANDLED CORRECT EXCEPTION TYPE (line " << line << ")";
        }
        out << std::endl;
    }


    static void register_test(i_test_case *test_case) {
        instance().tests[test_case->name()] = test_case;
    }

    static void run_all_tests() {
        for (const auto & test : instance().tests) {
            test.second->run();
        }
    }

    static bool run_test(const std::string& name) {
        auto it = instance().tests.find(name);
        if (it == instance().tests.end()) {
            return false;
        }
        it->second->run();
        return true;
    }

    static test_core & instance() {
        static test_core _core;
        return _core;
    }
};

struct simple_test_case : i_test_case {
    std::string _name;
    std::function<void()> routine;

    simple_test_case(std::string const& name, std::function<void()> routine) :
            _name(name), routine(routine) {
        test_core::register_test(this);
    }

    void run() override {
        cout_wrapper out;
        out << "TEST CASE: " << name() << std::endl;
        try {
            routine();
        } catch (std::exception e) {
            std::cerr << "Exception handled:\n" <<
                e.what() << std::endl;
        } catch (...) {
            std::cerr << "Undefined exception handled" << std::endl;
        }
        out << std::endl;
    }

    const std::string &name() override {
        return _name;
    }
};

struct test_case_with_timer : i_test_case {
    std::string _name;
    std::function<void()> routine;
    long long timeout_ms;

    test_case_with_timer(std::string const& name, 
            std::function<void()> routine, long long milliseconds) :
           _name(name), routine(routine), timeout_ms(milliseconds) {
        test_core::register_test(this);
    }

    void run() override {
        cout_wrapper out;
        out << "TEST CASE: " << name() << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
        try {
            routine();
        } catch (std::exception e) {
            std::cerr << "Exception handled:\n" <<
                e.what() << std::endl;
        } catch (...) {
            std::cerr << "Undefined exception handled" << std::endl;
        }
        auto end = std::chrono::high_resolution_clock::now();

        auto test_duration = std::chrono::duration_cast<
                        std::chrono::milliseconds>(end - start);
        auto duration_ms = test_duration.count();

        if (duration_ms < timeout_ms) {
            out << "Test ended in " << duration_ms << "ms" << std::endl;
        } else {
            out << "TEST CASE FAILED BY TIMEOUT: " << std::endl;
            out << "  Test ended in " << duration_ms << "ms" << std::endl;
            out << "  Timeout is "    << timeout_ms << "ms" << std::endl;
        }

        out << std::endl;
    }

    const std::string &name() override {
        return _name;
    }
};

# define _FT_CONCAT(a, b) a##b
# define _ADD_TC_PREFIX(_test_name) _FT_CONCAT(tc_, _test_name)
# define _ADD_TC_FUN_PREFIX(_test_name) _FT_CONCAT(tc_f_, _test_name)

// _name must be one word
// Example: TEST_CASE(some) {ASSERT_TRUE(42 == 42)}
# define TEST_CASE(_name)                 \
static void _ADD_TC_FUN_PREFIX(_name)();  \
static simple_test_case                   \
    _ADD_TC_PREFIX(_name)(#_name, &_ADD_TC_FUN_PREFIX(_name)); \
static void _ADD_TC_FUN_PREFIX(_name)()


// _name must be one word
// Example: TEST_CASE(some, 750) {std::this_thread::sleep(500);}
# define TEST_CASE_WITH_TIMER(_name, _milliseconds) \
static void _ADD_TC_FUN_PREFIX(_name)();            \
static test_case_with_timer                         \
  _ADD_TC_PREFIX(_name)(#_name, &_ADD_TC_FUN_PREFIX(_name), _milliseconds); \
static void _ADD_TC_FUN_PREFIX(_name)()



# define ASSERT_TRUE(_must_be_true) {auto __line = __LINE__; \
    bool __is_true = _must_be_true;  \
    test_core::register_assert_true( \
        __is_true,          \
        #_must_be_true,     \
        __line              \
    );                      \
                            \
    if (not (__is_true))    \
        return;             \
}

# define ASSERT_EXCEPTION(_expr, _exception) {auto __line = __LINE__;       \
    bool __the_exception_handled = false;                                   \
    try {                                                                   \
        _expr;                                                              \
    } catch (_exception e) {                                                \
        __the_exception_handled = true;                                     \
    } catch (...) {}                                                        \
    test_core::register_exception_handling(__the_exception_handled, __line);\
                                      \
    if (not __the_exception_handled)  \
        return;                       \
}

#endif // FT_SIMPLE_TEST_LIB_HPP