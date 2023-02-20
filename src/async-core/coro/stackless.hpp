#ifndef FT_STACKLESS_CORO_HPP
# define FT_STACKLESS_CORO_HPP

# include "../future/future.hpp"
# include <coroutine>
# include "../util/util.hpp"

namespace ft {

template <typename T>
struct CoroutinePromise {
    std::optional<ft::Promise<T>> promise;

    auto get_return_object() {
        auto [f, p] = ft::futures::make_contract<T>();
        promise.emplace(std::move(p));
        return std::move(f);
    }

    void return_value(T value) {
        std::move(*promise).set(std::move(value));
    }

    void set_exception(std::exception_ptr e) {
        (void) e;
    }

    void unhandled_exception() {
    }

    std::suspend_never initial_suspend() noexcept {
        return {};
    }

    std::suspend_never final_suspend() noexcept {
        return {};
    }
};

} // namespace ft

template <typename R, typename... Args>
struct std::coroutine_traits<ft::Future<R>, Args...> {
    using promise_type = ft::CoroutinePromise<R>;
};

// //////////////////////////////////////////////////////////////////////

// namespace ft {

// template <>
// struct CoroutinePromise<void> {
//     std::optional<Promise<Void>> promise;

//     auto get_return_object() {
//         auto [f, p] = ft::futures::make_contract<Void>();
//         promise.emplace(std::move(p));
//         return std::move(f);
//     }

//     std::suspend_never initial_suspend() noexcept {
//         return {};
//     }

//     std::suspend_never final_suspend() noexcept {
//         return {};
//     }

//     void set_exception(std::exception_ptr e) {
//         (void) e;
//     }

//     void unhandled_exception() {
//     }

//     void return_void() {
//         std::move(*promise).set(Void{});
//     }
// };

// } // namespace ft


// template <typename... Args>
// struct std::coroutine_traits<ft::Future<void>, Args...> {
//     using promise_type = ft::CoroutinePromise<void>;
// };

namespace ft {

template <typename T>
class FutureAwaiter {
public:
    FutureAwaiter(Future<T>&& f) : future(std::move(f)) {}

    bool await_ready() {
        return future.try_get(result);
    }

    void await_suspend(std::coroutine_handle<> h) {
        std::move(future).on_complete([this, h](T res) mutable {
            result.emplace(std::move(res));
            h.resume();
        });
    }

    auto await_resume() {
        return std::move(result.value());
    }

private:
    Future<T> future;
    std::optional<T> result = std::nullopt;
};

template <typename T>
auto operator co_await(Future<T>&& f) {
    return FutureAwaiter<T>(std::move(f));
}

} // namespace ft

#endif // FT_STACKLESS_CORO_HPP
