#include <async-core/mpmc-queue/mpmc_queue.hpp>
#include <simple_test_lib.hpp>

#include <iostream>
#include <thread>

using namespace ft::mpmc;

TEST_CASE_WITH_TIMER(channel, 5200) {
    auto pair = channel<int>();
    auto sender   = std::move(pair.first);
    auto receiver = std::move(pair.second);

    std::thread t([sender = std::move(sender)]() mutable {
        sender.send(100);
        sender.send(100);
        sender.send(100);
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    });
    t.detach();

    ASSERT_TRUE(receiver.recv() == 100);
    ASSERT_TRUE(receiver.recv() == 100);
    ASSERT_TRUE(receiver.recv() == 100);
    ASSERT_EXCEPTION(receiver.recv(), ft::mpmc::channel_closed)
    ASSERT_EXCEPTION(receiver.recv(), ft::mpmc::channel_closed)
}
