#ifndef FT_FUTURE_HPP
# define FT_FUTURE_HPP

# include <mutex>
# include <condition_variable>
# include <optional>
# include <memory>
# include <utility>
# include <functional>

# include "../util/move_only_func.hpp"
# include "../executors/IExecutor.hpp"
# include "../executors/same_thread.hpp"
# include "detail/_promise_container.hpp"
# include "detail/_shared_state.hpp"

namespace ft {

template <typename T>
class Promise;

template <typename T>
class Future;

namespace futures {

template <typename T>
std::pair<ft::Future<T>, ft::Promise<T>> make_contract();

} // namespace futures


template <typename T>
class Future {
    // shared state with Promise
    std::shared_ptr<detail::_state<T>> _st;

    // constructor must be private
    // Future may be build only with make_contract function
    Future(std::shared_ptr<detail::_state<T>> _st) : _st(_st) {};
    template <class U>
    friend std::pair<Future<U>, Promise<U>> futures::make_contract();

    Future(const Future & ) = delete;
public:
    using ValueType = T;

    Future(Future &&  other) = default;

    Future<T>&  with_executor(IExecutor *executor) {
        _st->set_executor(executor);
        return this;
    }

    std::optional<T> try_get() {
        return _st->try_get();
    }

    T block_get() {
        return _st->get_value();
    }

    void on_complete(::detail::unique_function<void(T)> callback) {
        _st->set_callback(std::move(callback));
    }

    // TODO: rewrite with concept
    // Get Fn(T) -> U and return Future<U>
    template <typename F, typename R =
            decltype(std::declval<F>()(std::declval<T>()))>
    Future<R> map(F&& func) {
        auto fut_prom = futures::make_contract<R>();
        auto future = std::move(fut_prom.first);
        
        this->on_complete([promise = std::move(fut_prom.second), 
                    func=std::move(func)](auto val) mutable {
            promise.set(std::move(func(std::move(val))));
        });
        return future;
    }

    // TODO: rewrite with concept
    // Get Fn(T) -> Future<U> and return Future<U>
    template <typename F, typename R = typename 
            decltype(std::declval<F>()(std::declval<T>()))::ValueType>
    Future<R> flatmap(F func) {
        auto fut_prom = futures::make_contract<R>();
        auto future   = std::move(fut_prom.first);
        auto promise  = std::move(fut_prom.second);
        
        this->on_complete([promise = std::move(promise), 
                func=std::move(func)](auto val) mutable {
            
            auto next_fut = func(std::move(val));
            next_fut.on_complete([promise = std::move(promise)]
                    (auto val2) mutable {
                promise.set(std::move(val2));
            });
        });
        return future;
    }
};


template <typename T>
class Promise {
    // shared state with Promise
    std::shared_ptr<detail::_state<T>> _st;

    // constructor must be private
    // Future may be build only with make_contract function
    Promise(std::shared_ptr<detail::_state<T>> _st) : _st(_st) {};
    template <class U>
    friend std::pair<Future<U>, Promise<U>> futures::make_contract();

public:
    using ValueType = T;

    Promise(const Promise & ) = default;
    Promise(Promise &&  ) = default;

    void set(T value) {
        _st->set_value(std::move(value));
    }
};

namespace futures {

template <typename T>
static Future<T> from_val(T val) {
    auto [future, promise] = make_contract<T>();
    promise.set(std::move(val));
    return std::move(future);
}

template <typename T>
std::pair<Future<T>, Promise<T>> make_contract() {
    auto _st = std::make_shared<detail::_state<T>>();
    auto future = Future<T>(_st);
    auto promise = Promise<T>(_st);
    return std::make_pair(std::move(future), std::move(promise));
}

template <typename InputIterator, 
        typename V = typename std::iterator_traits<
            InputIterator>::value_type::ValueType>
Future<std::vector<V>>
    collect(InputIterator it, InputIterator end, int n_expected) {
    
    auto fut_prom = make_contract<std::vector<V>>();
    auto future = std::move(fut_prom.first);
    auto promise = std::move(fut_prom.second);

    auto scoped_fut_prom = ft::detail::make_cont_contract<V>(n_expected);
    auto f_with_cont = std::move(scoped_fut_prom.first);
    auto p_with_cont = std::move(scoped_fut_prom.second);

    for (; it != end; ++it) {
        it->on_complete(
            [container=p_with_cont](auto value) mutable {
                container.add(value);
            }
        );
    }

    f_with_cont.on_complete(
        [promise=std::move(promise)](auto values) mutable {
            promise.set(values);
        }
    );

    return future;
}

template <typename InputIterator, 
        typename V = typename std::iterator_traits<
            InputIterator>::value_type::ValueType>
Future<V> fastest(InputIterator it, InputIterator end) {
    auto fut_prom = make_contract<V>();
    auto future = std::move(fut_prom.first);
    auto promise = std::move(fut_prom.second);

    for (; it != end; ++it) {
        it->on_complete(
            [promise=promise](auto value) mutable {
                promise.set(value);
            }
        );
    }
    return future;
}


} // mamespace futures


} // namespace ft

#endif // FT_FUTURE_HPP