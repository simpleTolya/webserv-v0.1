#ifndef FT_IO_CONCEPTS_HPP
# define FT_IO_CONCEPTS_HPP

# include "../../util/Result.hpp"
# include "../../executors/IExecutor.hpp"
# include "EventLoop.hpp"

namespace ft::io {

template<typename R, typename Error>
concept Read =
    requires(R reader, unsigned char* buf, size_t buf_size) {
        requires std::same_as<Error, typename R::Error>;
        { reader.read_part(buf, buf_size) } -> 
                        std::same_as<Res<size_t, Error>>;
    };

template<typename W, typename Error>
concept Write =
    requires(W writer, const unsigned char* buf, size_t buf_size) {
        requires std::same_as<Error, typename W::Error>;
        { writer.write_part(buf, buf_size) } -> 
                        std::same_as<Res<size_t, Error>>;
    };

template<typename R>
concept AsyncRead = 
    requires(R reader, IExecutor *executor, Handler callback) {
        reader.when_readable(callback, executor);
    };

template<typename W>
concept AsyncWrite = 
    requires(W writer, IExecutor *executor, Handler callback) {
        writer.when_writable(callback, executor);
    };

} // namespace ft::io

#endif // FT_IO_CONCEPTS_HPP