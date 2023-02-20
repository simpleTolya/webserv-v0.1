#ifndef FT_THREAD_POOL_HPP
# define FT_THREAD_POOL_HPP

# include <thread>
# include <async-core/mpmc-queue/mpmc_queue.hpp>
# include "IExecutor.hpp"
# include <async-core/util/util.hpp>

namespace ft {

class StaticThreadPool : public IExecutor {
    class Worker {
        std::thread _thread;
        
        Worker(std::thread thread) :
            _thread(std::move(thread)) {}

        public:
            static Worker create(mpmc::Receiver<std::optional<Task>> tasks) {
                std::thread worker([
                        tasks = std::move(tasks)
                    ] () mutable {
                        while (true) {
                            auto task = std::move(tasks.recv());
                            if (not task.has_value())
                                break ;
                            task.value()();
                        }
                });
                return Worker(std::move(worker));
            }

            void join() {
                _thread.join();
            }
    };

    mpmc::Sender<std::optional<Task>>  to_workers;
    std::vector<Worker> workers;

    StaticThreadPool(
        mpmc::Sender<std::optional<Task>> sender,
        mpmc::Receiver<std::optional<Task>> receiver,
        unsigned thread_count) : to_workers(sender) 
    {
        for (unsigned i = 0; i < thread_count; ++i) {
            workers.emplace_back(
                std::move(
                    Worker::create(
                        receiver
                    )
                )
            );
        }
    }
public:
    StaticThreadPool(const StaticThreadPool &) = delete;
    StaticThreadPool(StaticThreadPool &&) = default;

    static StaticThreadPool create(unsigned thread_count) {
        auto [sender, receiver] = mpmc::channel<std::optional<Task>>();
        return std::move(StaticThreadPool(sender, receiver, thread_count));
    }

    static StaticThreadPool create_default() {
        auto [sender, receiver] = mpmc::channel<std::optional<Task>>();
        auto thread_cnt = std::thread::hardware_concurrency();
        if (thread_cnt == 0)
            thread_cnt = 4;
        return std::move(StaticThreadPool(sender, receiver, thread_cnt));
    }

    void execute(Task && task) override {
        to_workers.send(std::move(task));
    }

    void shutdown() {
        for (size_t i = 0; i < workers.size(); ++i)
            to_workers.send(std::nullopt);

        for (auto & worker : workers)
            worker.join();

        workers.clear();
    }

    ~StaticThreadPool() {
        shutdown();
    }
};

} // namespace ft

#endif // FT_THREAD_POOL_HPP