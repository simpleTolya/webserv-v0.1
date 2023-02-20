#ifndef FT_SOCKET_HPP
# define FT_SOCKET_HPP

# include "ExecutionContext.hpp"
# include "async-core/io/error/Result.hpp"
# include <unistd.h>

namespace ft::io {

class Socket {
    int fd;
    ExecutionContext *context;

public:
    using Error = ft::io::Error;

    explicit Socket(int fd, ExecutionContext *context) : 
        fd(fd), context(context) {}
    Socket(const Socket &) = delete;
    Socket(Socket && other) {
        context = other.context;
        fd = other.fd;
        other.fd = 0;
    }

    static Result<Socket> conn_tcp_serv(const char *ip,
                        uint16_t port, ExecutionContext *);

    ~Socket() {
        close();
    }

    void close() {
        if (fd != 0) {
            ::close(fd);
            fd = 0;
        }
    }

    int freopen(int fd) {
        return dup2(this->fd, fd);
    }

    Result<Void> read_vec(Data &buf);

    inline Result<Data> read_vec() {
        Data data;
        auto v = read_vec(data);
        return v.map([data=std::move(data)](Void _){return data;});
    }

    // low-level
    inline Result<size_t> read_part(unsigned char *buf, size_t buf_size) {
        auto cnt = read(fd, buf, buf_size);
        if (cnt == -1) {
            if (errno == EAGAIN)
                return Result<size_t>(0);
            return Result<size_t>(from_errno(errno));
        }
        return Result<size_t>(cnt);
    }

    Result<size_t> write_all(const Data &data);

    // low-level
    inline Result<size_t> write_part(
                    const unsigned char *buf, size_t buf_size) {
        
        auto cnt = write(fd, buf, buf_size);
        if (cnt == -1) {
            if (errno == EAGAIN)
                return Result<size_t>(0);
            return Result<size_t>(from_errno(errno));
        }
        return Result<size_t>(cnt);
    }

    inline void when_readable(Handler callback) {
        context->add_handler_for_event(
            Event(fd, Event::TO_READ),
            std::move(callback)
        );
    }

    inline void when_writable(Handler callback) {
        context->add_handler_for_event(
            Event(fd, Event::TO_WRITE),
            std::move(callback)
        );
    }
};

} // namespace ft::io

#endif // FT_SOCKET_HPP