#ifndef FT_MPMC_QUEUE_HPP
# define FT_MPMC_QUEUE_HPP

# include <mutex>
# include <condition_variable>
# include <optional>
# include <memory>
# include <utility>
# include <functional>
# include <atomic>

# include "detail/forward_list.hpp"
# include <async-core/util/Result.hpp>
# include <async-core/util/util.hpp>

namespace ft {

namespace mpmc {

struct closed  {
};

} // namespace mpmc

namespace detail {

template <typename T, typename Allocator = std::allocator<T>>
class UnboundedMPMCQueue {
    ft::detail::forward_list<T, Allocator>  _queue;
    std::mutex  _mutex;
    std::condition_variable  _not_empty;
    unsigned _consumder_count;
    unsigned _producer_count;

public:
    using ValueType = T;
    using AllocatorType = Allocator;

    UnboundedMPMCQueue() : 
        _queue(), _mutex(), _not_empty(),
        _consumder_count(1), _producer_count(1) {}

    void _incr_consumer_cnt() {
        std::lock_guard lock(_mutex);
        _consumder_count += 1; 
    }

    void _decr_consumer_cnt() {
        std::lock_guard lock(_mutex);
        _consumder_count -= 1;
    }

    void _incr_producer_cnt() {
        std::lock_guard lock(_mutex);
        _producer_count += 1;
    }

    void _decr_producer_cnt() {
        std::lock_guard lock(_mutex);
        _producer_count -= 1;
        if (_producer_count == 0) {
            _not_empty.notify_all();
        }
    }

    ft::Res<Void, ft::mpmc::closed> push(T value) {
        using _Result = ft::Res<Void, ft::mpmc::closed>;
        std::lock_guard lock(_mutex);
        if (_consumder_count == 0) {
            return _Result(ft::mpmc::closed{});
        }
        _queue.push_back(std::move(value));
        _not_empty.notify_one();
        return _Result(Void{});
    }

    ft::Res<T, ft::mpmc::closed> pop() {
        using _Result = ft::Res<T, ft::mpmc::closed>;
        std::unique_lock lock(_mutex);
        while (_queue.is_empty()) {
            if (_producer_count == 0) {
                return _Result(ft::mpmc::closed{});
            }
            _not_empty.wait(lock);
        }
        return _Result(std::move(_queue.pop_front().value()));
    }

    ft::Res<std::optional<T>, ft::mpmc::closed> try_pop() {
        using _Result = ft::Res<std::optional<T>, ft::mpmc::closed>;
        std::lock_guard lock(_mutex);
        if (_queue.is_empty() && _producer_count == 0) {
            return _Result(ft::mpmc::closed{});
        }
        return _Result(std::move(_queue.pop_front()));
    }
};

} // namespace detail

namespace mpmc {

template<typename T, typename Allocator>
class Sender;

template<typename T, typename Allocator>
class Receiver;

template<typename T, typename Allocator = std::allocator<T>>
class Sender {
    std::shared_ptr<detail::UnboundedMPMCQueue<T, Allocator>> _queue;

    Sender(std::shared_ptr<detail::UnboundedMPMCQueue<T, Allocator>> queue) :
        _queue(queue) {}

    template<typename U, typename A> 
    friend std::pair<Sender<U, A>, Receiver<U, A>> channel();

public:

    Sender(const Sender &other) : _queue(other._queue) {
        if (_queue != nullptr)
            _queue->_incr_producer_cnt();
    }

    Sender(Sender &&) = default;

    Sender & operator=(const Sender &) = delete;
    Sender & operator=(Sender &&) = delete;

    ~Sender() {
        if (_queue != nullptr)
            _queue->_decr_producer_cnt();
    }

    // template <typename ...Args>
    // void send(Args&&... value) {
    //     _queue->push(std::forward<Args>(value)...);
    // }

    ft::Res<Void, closed> send(T value) {
        return _queue->push(std::move(value));
    }
};

template<typename T, typename Allocator = std::allocator<T>>
class Receiver {
    std::shared_ptr<detail::UnboundedMPMCQueue<T, Allocator>> _queue;

    Receiver(std::shared_ptr<detail::UnboundedMPMCQueue<T, Allocator>> queue):
        _queue(queue) {}

    template<typename U, typename A> 
    friend std::pair<Sender<U, A>, Receiver<U, A>> channel();

public:
    Receiver(const Receiver &other) : _queue(other._queue) {
        _queue->_incr_consumer_cnt();
    }

    Receiver(Receiver &&) = default;

    Receiver & operator=(const Receiver &) = delete;
    Receiver & operator=(Receiver &&) = delete;

    ~Receiver() {
        if (_queue != nullptr)
            _queue->_decr_consumer_cnt();
    }

    ft::Res<T, closed> recv() {
        return _queue->pop();
    }

    ft::Res<std::optional<T>, closed> try_recv() {
        return _queue->try_pop();
    }
};

template<typename T, typename Allocator = std::allocator<T>> 
std::pair<Sender<T, Allocator>, Receiver<T, Allocator>> channel() {
    auto _queue = std::make_shared<
            ft::detail::UnboundedMPMCQueue<T, Allocator>>();
    auto sender = Sender(_queue);
    auto receiver = Receiver(_queue);
    return std::make_pair<>(std::move(sender), std::move(receiver));
}


} // namespace mpmc

}

#endif // FT_MPMC_QUEUE_HPP