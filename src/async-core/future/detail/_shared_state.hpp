#ifndef FT_FUTURE_SHARED_STATE_HPP
# define FT_FUTURE_SHARED_STATE_HPP

# include <mutex>
# include <condition_variable>
# include <optional>
# include <memory>
# include <utility>
# include <functional>

# include "../../util/move_only_func.hpp"
# include "../../executors/IExecutor.hpp"
# include "../../executors/same_thread.hpp"

namespace ft::detail {

    template <typename T>
    class _state {
        std::optional<T>   _value;
        std::optional<::detail::unique_function<void(T)>>   _callback;
        IExecutor *_executor;
        mutable std::mutex _mutex;
        mutable std::condition_variable _not_empty;
    private:
        void _call_by_executor() {
            _executor->execute([
                value = std::move(_value.value()),
                callback = std::move(_callback.value())
            ] () mutable { callback(std::move(value)); });
        }
    public:
        _state() : 
            _value(std::nullopt), 
            _callback(std::nullopt),
            _executor(ft::same_thread::_()),
            _mutex(), _not_empty() {}

        void set_executor(IExecutor *executor) {
            std::lock_guard lock(_mutex);
            _executor = executor;
        }

        bool try_get(std::optional<T> &opt) {
            std::lock_guard lock(_mutex);
            if (_value.has_value()) {
                opt.emplace(std::move(_value.value()));
                return true;
            }
            return false;
        }

        void set_value(T value) {
            std::lock_guard lock(_mutex);
            _value.emplace(std::move(value));
            if (_callback.has_value()) {
                _call_by_executor();
                _callback = std::nullopt;
            } else {
                _not_empty.notify_one();
            }
        }

        T get_value() {
            std::unique_lock lock(_mutex);
            while (not _value.has_value()) {
                _not_empty.wait(lock);
            }
            return std::move(_value.value());
        }

        void set_callback(::detail::unique_function<void(T)> callback) {
            std::lock_guard lock(_mutex);
            _callback.emplace(std::move(callback));
            if (_value.has_value())
                _call_by_executor();
        }
    };

} // namespace ft::detail


# endif // FT_FUTURE_SHARED_STATE_HPP