#include <async-core/future/future.hpp>

#include <vector>
#include <chrono>
#include <thread>
#include <iostream>

#include "../../simple_test_lib.hpp"

ft::Future<int> long_calc(int val, int milliseconds) {
    auto fut_promise = ft::futures::make_contract<int>();
    std::thread t([val=val, promise=std::move(fut_promise.second), time=milliseconds]() mutable {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        promise.set(val + 1);
    });
    t.detach();
    return std::move(fut_promise.first);
}

ft::Future<int> calc_sec(int val) {
    return long_calc(val, 1000);
}


TEST_CASE_WITH_TIMER(future_map, 1500) {    
    bool is_ok = false;
    auto plus_42 = [](int val){return val + 42;};

    calc_sec(0) // returns 1
            .map(plus_42)
            .map(plus_42)
            .map(plus_42)
            .on_complete([&is_ok](int var){
                if (var == 127) {
                    is_ok = true;
                }
            });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1250));
    ASSERT_TRUE(is_ok)
}


TEST_CASE_WITH_TIMER(future_flatmap, 4250) {    
    auto val = calc_sec(0)
            .flatmap(calc_sec)
            .flatmap(calc_sec)
            .flatmap(calc_sec)
            .block_get();

    ASSERT_TRUE(val == 4)
}


TEST_CASE_WITH_TIMER(collect1, 1250) { 
    std::vector<decltype(long_calc(0, 0))> futures;
    
    futures.push_back(std::move(long_calc(0, 1000)));
    futures.push_back(std::move(long_calc(1, 2000)));
    futures.push_back(std::move(long_calc(2, 3000)));

    std::vector<int> values = 
            ft::futures::collect(futures.begin(), futures.end(), 1)
                .block_get();

    
    ASSERT_TRUE(values[0] == 1)
}

TEST_CASE_WITH_TIMER(collect2, 2250) {
    std::vector<decltype(long_calc(0, 0))> futures;
    
    futures.push_back(std::move(long_calc(0, 1000)));
    futures.push_back(std::move(long_calc(1, 2000)));
    futures.push_back(std::move(long_calc(2, 3000)));

    std::vector<int> values = 
            ft::futures::collect(futures.begin(), futures.end(), 2)
                .block_get();

    ASSERT_TRUE(values[0] == 1)
    ASSERT_TRUE(values[1] == 2)
}

TEST_CASE_WITH_TIMER(collect_all, 3250) {
    std::vector<decltype(long_calc(0, 0))> futures;
    
    futures.push_back(std::move(long_calc(0, 1000)));
    futures.push_back(std::move(long_calc(1, 2000)));
    futures.push_back(std::move(long_calc(2, 3000)));

    std::vector<int> values = 
            ft::futures::collect(futures.begin(), futures.end(), 3)
                .block_get();

    ASSERT_TRUE(values[0] == 1)
    ASSERT_TRUE(values[1] == 2)
    ASSERT_TRUE(values[2] == 3)
}


TEST_CASE_WITH_TIMER(make_contract, 1250) {
    auto [future, promise] = ft::futures::make_contract<int>();

    auto _fut = std::move(future);
    auto _pr = std::move(promise);

    auto t = std::thread([promise=std::move(_pr)]() mutable {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        promise.set(10);
    });
    t.detach();
    ASSERT_TRUE(_fut.block_get() == 10)
}
