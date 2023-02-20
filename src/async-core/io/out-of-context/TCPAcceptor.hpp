#ifndef FT_TCPACCEPTOR_HPP
# define FT_TCPACCEPTOR_HPP

# define LISTEN_BACKLOG 512

# include "ExecutionContext.hpp"
# include "Socket.hpp"

namespace ft::io {

struct InAddrInfo {
    char ip[18] = {0};
    int port = 0;
};

class TCPAcceptor {
    int fd;
    ExecutionContext *context;
public:
    using Error = ft::io::Error;
    static Result<TCPAcceptor> local_with_port(
                    int port, ExecutionContext *context);

    explicit TCPAcceptor(int fd, ExecutionContext *context) : 
        fd(fd), context(context) {}
    TCPAcceptor(const TCPAcceptor &) = delete;
    TCPAcceptor(TCPAcceptor &&other) {
        context = other.context;
        fd = other.fd;
        other.fd = 0;
    }

    ~TCPAcceptor() {
        close();
    }

    void close() {
        if (fd != 0) {
            ::close(fd);
            fd = 0;
        }
    }

    Result<std::pair<Socket, InAddrInfo>> accept_conn();

    inline void when_acceptable(Handler callback) {
        context->add_handler_for_event(
            Event(fd, Event::TO_ACCEPT),
            std::move(callback)
        );
    }
};

} // namespace ft::io

#endif // FT_TCPACCEPTOR_HPP