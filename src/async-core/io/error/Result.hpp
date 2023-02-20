#ifndef FT_IO_RESULT_HPP
# define FT_IO_RESULT_HPP

# include <async-core/util/Result.hpp>

namespace ft::io {

enum class Error {
    UNDEFINED = 0,

    // a connection has been aborted
    CONNECTION_ABORTED = 1,

    // the system call was interrupted by a signal
    INTERRUPTED = 2,

    // the per-process limit on the number of open fd has been reached
    OPEN_FD_LIMIT_PROC = 3,

    // the system-wide limit on the total number of open files has been reached
    OPEN_FD_LIMIT_SYS = 4,

    // Not enough free memory
    BUF_OUT_MEMORY = 5,
                
    LOGIC = 6,

    PORT_ALREADY_USE = 7,

    AGAIN = 8,

    CONNECTION_CLOSED = 9
};

Error        from_errno(int err);
const char * error_description(Error err);

template <typename T>
using Result = ft::Res<T, Error>;

} // namespace ft::io


#endif // FT_IO_RESULT_HPP