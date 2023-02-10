#ifndef FT_PROMISE_CONTAINER_HPP
# define FT_PROMISE_CONTAINER_HPP

# include <mutex>
# include <condition_variable>
# include <optional>
# include <memory>
# include <utility>
# include <functional>

# include "../../util/move_only_func.hpp"

namespace ft {

namespace detail {

template <typename T, typename Container>
class ContainerPromise;

template <typename T, typename Container>
class ContainerFuture;

template <typename T, typename Container = std::vector<T> >
std::pair<
    ft::detail::ContainerFuture<T, Container>, 
    ft::detail::ContainerPromise<T, Container>
        > make_cont_contract(unsigned required_amount);

namespace {

template <typename T, typename Container >
class _shared_container {
    Container _values;
    unsigned  _required_amount;
    std::optional<::detail::unique_function<void(Container)>> _callback;
    mutable std::mutex _mutex;
    mutable std::condition_variable _not_empty;
public:
    _shared_container(unsigned required_amount) :
        _required_amount(required_amount),
        _callback(std::nullopt), _values(),
        _mutex(), _not_empty() {}

    void add_value(T value) {
        std::lock_guard lock(_mutex);
        if (_values.size() + 1 >= _required_amount) {

            _values.push_back(value);
            if (_callback.has_value()) {
                _callback.value()(_values);
                _callback = std::nullopt;
            }
            _not_empty.notify_one();
        } else {
            _values.push_back(value);
        }
    }

    Container get_value() {
        std::unique_lock lock(_mutex);
        while (_values.size() < _required_amount) {
            _not_empty.wait(lock);
        }
        return std::move(_values);
    }

    void set_callback(::detail::unique_function<void(Container)> callback) {
        std::lock_guard lock(_mutex);
        if (_values.size() >= _required_amount) {
            callback(_values);
        } else {
            _callback.emplace(std::move(callback));
        }
    }
};

} // noname namespace

template <typename T, typename Container>
class ContainerFuture {
    // shared state with Promise
    std::shared_ptr<_shared_container<T, Container>> _st;

    // constructor must be private
    // Future may be build only with make_contract function
    ContainerFuture(std::shared_ptr<_shared_container<T, Container>> _st) 
        : _st(_st) {};

    template <class U, class C>
    friend std::pair<
        ft::detail::ContainerFuture<U, C>, 
        ft::detail::ContainerPromise<U, C>
            > make_cont_contract(unsigned required_amount);

    ContainerFuture(const ContainerFuture & ) = delete;
public:
    using ValueType = T;

    ContainerFuture(ContainerFuture &&  other) = default;

    T block_get() {
        return _st->get_value();
    }

    void on_complete(::detail::unique_function<void(Container)> callback) {
        _st->set_callback(std::move(callback));
    }
};


template <typename T, typename Container>
class ContainerPromise {
    // shared state with Promise
    std::shared_ptr<_shared_container<T, Container>> _st;

    // constructor must be private
    // Future may be build only with make_contract function
    ContainerPromise(std::shared_ptr<_shared_container<T, Container>> _st)
     : _st(_st) {};
    
    template <class U, class C>
    friend std::pair<
        ft::detail::ContainerFuture<U, C>, 
        ft::detail::ContainerPromise<U, C>
            > make_cont_contract(unsigned required_amount);

public:
    using ValueType = T;

    ContainerPromise(const ContainerPromise & ) = default;
    ContainerPromise(ContainerPromise &&  ) = default;

    void add(T value) {
        _st->add_value(value);
    }
};

template <typename T, typename Container = std::vector<T> >
std::pair<
    ft::detail::ContainerFuture<T, Container>, 
    ft::detail::ContainerPromise<T, Container>
        > make_cont_contract(unsigned required_amount)
{
    auto _st = std::make_shared<_shared_container<T, Container>>(required_amount);
    auto future = ContainerFuture(_st);
    auto promise = ContainerPromise(_st);
    return std::make_pair(std::move(future), std::move(promise));
}

} // namespace detail

} // namespace ft


#endif // FT_PROMISE_CONTAINER_HPP