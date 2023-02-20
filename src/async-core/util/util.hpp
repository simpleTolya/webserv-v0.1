#ifndef FT_UTIL_HPP
# define FT_UTIL_HPP

# define FT_BUFFER_SIZE 4096

namespace ft {

extern thread_local unsigned char buffer[FT_BUFFER_SIZE];

struct Void {};

}

#endif // FT_UTIL_HPP