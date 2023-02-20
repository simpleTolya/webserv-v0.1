// #include <async-core/future/future.hpp>
// #include <async-core/coro/stackless.hpp>

// #include <vector>
// #include <chrono>
// #include <thread>
// #include <iostream>

// #include "../../simple_test_lib.hpp"

// ft::Future<int> long_calc(int val, int milliseconds) {
//     auto fut_promise = ft::futures::make_contract<int>();
//     std::thread t([val=val, promise=std::move(fut_promise.second), time=milliseconds]() mutable {
//         std::this_thread::sleep_for(std::chrono::milliseconds(time));
//         promise.set(val + 1);
//     });
//     t.detach();
//     return std::move(fut_promise.first);
// }

// ft::Future<int> calc_sec(int val) {
//     return long_calc(val, 1000);
// }


// TEST_CASE_WITH_TIMER(future_map, 1500) {    
//     bool is_ok = false;
//     auto plus_42 = [](int val){return val + 42;};

//     int val = co_await long_calc(1, 1000);
    
//     std::this_thread::sleep_for(std::chrono::milliseconds(1250));
//     ASSERT_TRUE(is_ok)
// }
