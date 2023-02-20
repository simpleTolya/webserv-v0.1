#ifndef FT_RES_HPP
# define FT_RES_HPP

# include <variant>
# include <type_traits>
# include <optional>

namespace ft {

template <typename T, typename E>
class Res
{
    std::variant<T, E> _res;
    bool _is_ok;

    Res(std::variant<T, E> res, bool is_ok) :
        _res(std::move(res)), _is_ok(is_ok) {}
public:
    using ValueType = T;
    using ErrorType = E;

    explicit Res(T val) : _res(std::move(val)),
        _is_ok(true) {}

    explicit Res(E err) : _res(std::move(err)),
        _is_ok(false) {}
    
    bool is_ok() const noexcept {
        return _is_ok;
    }

    bool is_err() const noexcept {
        return !_is_ok;
    }

    T& get_val() {
        return std::get<T>(_res);
    }

    E& get_err() {
        return std::get<E>(_res);
    }

    // F is Fn(T) -> R
    template <typename F, typename R =
            decltype(std::declval<F>()(std::declval<T>()))>
    Res<R, E> map(F&& func) {

        if (_is_ok) {
            auto& good = std::get<T>(_res);
            return Res<R, E>(func(good));
        }
        return Res<R, E>(std::move(std::get<E>(_res)));
    }

    // F is Fn(T) -> Res<R, E>
    template <typename F, typename R = typename 
            decltype(std::declval<F>()(std::declval<T>()))::ValueType>
    Res<R, E>   flat_map(F&& func) {
        
        if (_is_ok) {
            auto& good = std::get<T>(_res);
            return func(good);
        }
        return Res<R, E>(std::move(std::get<E>(_res)));
    }

    T   or_default(T default_val) {
        if (_is_ok)
            return std::get<T>(_res);
        return default_val;
    }
    
    std::optional<T> ok() {
        if (_is_ok)
            return std::move(std::get<T>(_res));
        return std::nullopt;
    }

    std::optional<T> err() {
        if (not _is_ok)
            return std::move(std::get<E>(_res));
        return std::nullopt;
    }
};


// TODO prototype (exception -> Result)
// template <typename F, typename... Args,
//     typename R = decltype(std::declval<F>()((std::declval<Args>)...))>
// Res<R, std::exception> wrapped_in_res(F&& f, Args&&... args) {
//     try {
//         auto val = f(std::forward<Args>(args)...);
//         return Res(val);
//     } catch (std::exception e) {
//         return Res(e);
//     } catch 
// }


} // namespace ft


#endif // FT_RES_HPP