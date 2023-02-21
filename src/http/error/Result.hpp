#ifndef FT_HTTP_ERROR_HPP
# define FT_HTTP_ERROR_HPP

# include <async-core/util/Result.hpp>
# include <async-core/io/error/Result.hpp>

namespace ft::http {

struct Error {
    enum _err {
        HTTP_REQUEST_PARSE,
        HTTP_RESPONSE_PARSE,
        UNDEFINED,
        INCOMPLETE,
        INVALID_STATUC_CODE,
        IO_ERROR
    } _http_err = UNDEFINED;

    io::Error _io_err = io::Error::UNDEFINED;

    Error(io::Error err) :  _http_err(IO_ERROR), _io_err(err) {}
    Error(_err err) : _http_err(err) {}
};

template <typename T>
using Result = Res<T, Error>;


} // namespace ft::http


#endif // FT_HTTP_ERROR_HPP