#ifndef FT_IO_CONCEPTS_HPP
# define FT_IO_CONCEPTS_HPP

# include "../../util/Result.hpp"
// # include "../../executors/IExecutor.hpp"

namespace ft::io {

// using Handler = ::detail::unique_function<void(Event::EventType)>;

template<typename R, typename Error>
concept Read =
    requires(R reader, unsigned char* buf, size_t buf_size) {
        { R::Error } -> std::same_as<Error>;
        { reader.read_part(buf, buf_size) } -> 
                        std::same_as<Res<size_t, Error>>;
        // { reader.when_readable(callback, executor) } -> ; TODO
    };

template<typename W, typename Error>
concept Write =
    requires(W writer, const unsigned char* buf, size_t buf_size) {
        { W::Error } -> std::same_as<Error>;
        { writer.write_part(buf, buf_size) } -> 
                        std::same_as<Res<size_t, Error>>;
        // { reader.when_readable(callback, executor) } -> ; TODO
    };


}

#endif // FT_IO_CONCEPTS_HPP