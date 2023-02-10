#ifndef FT_TCPACCEPTOR_HPP
# define FT_TCPACCEPTOR_HPP

# define LISTEN_BACKLOG 10

# include "EventLoop.hpp"
# include "Socket.hpp"

namespace ft::io {

class TCPAcceptor {
    int fd;

public:
    static Result<TCPAcceptor> local_with_port(int port);

    explicit TCPAcceptor(int fd) : fd(fd) {}
    TCPAcceptor(const TCPAcceptor &) = delete;
    TCPAcceptor(TCPAcceptor &&other) {
        fd = other.fd;
        other.fd = 0;
    }

    ~TCPAcceptor() {
        if (fd != 0)
            close(fd);
    }

    inline Result<Socket> accept_conn() {
        int conn_fd = accept(fd, 0, 0); // TODO USE sockaddr info
        if (conn_fd == -1) {
            return Result<Socket>(from_errno(errno));
        }
        return Result<Socket>(Socket(conn_fd));
    }

    inline void when_acceptable(Handler callback, 
            EventLoop *event_loop, IExecutor * executor) {
        
        event_loop->add_handler_for_event(
            Event(fd, Event::TO_ACCEPT),
            std::move(callback),
            executor
        );
    }
};

} // namespace ft::io

#endif // FT_TCPACCEPTOR_HPP