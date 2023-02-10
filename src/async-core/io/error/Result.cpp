#include "Result.hpp"
#include <errno.h>

namespace ft::io {

const char * error_description(Error err) {
    static const char* description[] = {
        "undefined error",
        "a connection has been aborted",
        "the system call was interrupted by a signal",
        "the per-process limit on the number of open fd has been reached",
        "the system-wide limit on the total number of open files has been reached",
        "Not enough free memory",
        "logic error",
        "given port already in use",
        "eagain",
        "connection was closed"
    };
    return description[static_cast<int>(err)];
}

Error   from_errno(int err) {
    switch (err) {
        case ECONNABORTED: return Error::CONNECTION_ABORTED;
        case EINTR: return Error::INTERRUPTED;
        case EMFILE: return Error::OPEN_FD_LIMIT_PROC;
        case ENFILE: return Error::OPEN_FD_LIMIT_SYS;
        case ENOMEM: return Error::BUF_OUT_MEMORY;
        case EADDRINUSE: return Error::PORT_ALREADY_USE;
        case EAGAIN: return Error::AGAIN;

        case EINVAL: 
        case EBADF: 
        case ENOTSOCK: 
            return Error::LOGIC;

        default: return Error::UNDEFINED;
    }
}

} // namespace ft::io