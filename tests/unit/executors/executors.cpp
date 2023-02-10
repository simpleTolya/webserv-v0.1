#include <async-core/executors/same_thread.hpp>
#include <async-core/executors/StaticThreadPool.hpp>
#include <async-core/future/future.hpp>
#include "../../simple_test_lib.hpp"

#include <iostream>
#include <thread>

using namespace ft;

TEST_CASE(same_thread) {
    IExecutor * executor = same_thread::_();

    int a = 5;
    executor->execute([&a](){a += 5;});
    ASSERT_TRUE(a == 10)
}


TEST_CASE(static_thread_pool) {
    auto thread_pool = StaticThreadPool::create_default();

    auto in_pool = [&thread_pool](std::function<int()> int_getter){
        auto [future, promise] = futures::make_contract<int>();
        
        thread_pool.execute([
                promise=std::move(promise),
                task=std::move(int_getter)
                ]() mutable {
            int ans = task();
            promise.set(ans);
        });

        return std::move(future);
    };

    Future<int> ten = in_pool([](){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 
        return 10;
    });
    int res = ten.block_get();

    ASSERT_TRUE(res == 10)
}