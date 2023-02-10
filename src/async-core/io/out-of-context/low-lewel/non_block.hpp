#ifndef FT_IO_NON_BLOCK_HPP
# define FT_IO_NON_BLOCK_HPP

#include <fcntl.h>

namespace ft::io::detail {

inline int set_nonblock(int sock) {
    return fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
}

} // namespace ft::io::detail

#endif // FT_IO_NON_BLOCK_HPP