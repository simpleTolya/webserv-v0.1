#include <async-core/mpmc-queue/detail/forward_list.hpp>
#include <simple_test_lib.hpp>
#include <string>

TEST_CASE(forward_list) {

    ft::detail::forward_list<std::string> lst;
    ASSERT_TRUE(lst.is_empty());
    ASSERT_TRUE(lst.pop_front() == std::nullopt);

    lst.push_back("hello");
    ASSERT_TRUE(!lst.is_empty());
    ASSERT_TRUE(lst.pop_front().value() == "hello");
    ASSERT_TRUE(lst.pop_front() == std::nullopt);
    ASSERT_TRUE(lst.is_empty());


    for (int i=0; i<1000; ++i) {
        lst.push_back(std::to_string(i));
    }

    bool success = true;
    for (int i=0; i<1000; ++i) {
        if ( lst.pop_front().value() != std::to_string(i)) {
            success = false;
        }
    }
    ASSERT_TRUE(success);
    ASSERT_TRUE(lst.pop_front() == std::nullopt)
    ASSERT_TRUE(lst.is_empty())
}