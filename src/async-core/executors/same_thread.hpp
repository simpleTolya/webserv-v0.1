#ifndef FT_SAME_THREAD_HPP
# define FT_SAME_THREAD_HPP

# include <thread>
# include "../mpmc-queue/mpmc_queue.hpp"
# include "IExecutor.hpp"

namespace ft {

class same_thread : public IExecutor {
    same_thread() = default;
public:
    void execute(Task&& task) override {
        task();
    }

    // Getting instance as ft::same_thread::_()
    static same_thread *_() {
        static same_thread instance;
        return &instance;
    }
};

} // namespace ft

#endif // FT_SAME_THREAD_HPP