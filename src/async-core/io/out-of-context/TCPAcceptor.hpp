#ifndef FT_TCPACCEPTOR_HPP
# define FT_TCPACCEPTOR_HPP

# define LISTEN_BACKLOG 512

# include "EventLoop.hpp"
# include "Socket.hpp"

namespace ft::io {

struct InAddrInfo {
    char ip[18] = {0};
    int port = 0;
};

class TCPAcceptor {
    int fd;
    EventLoop *event_loop;
public:
    using Error = ft::io::Error;
    static Result<TCPAcceptor> local_with_port(
                    int port, EventLoop *event_loop);

    explicit TCPAcceptor(int fd, EventLoop *event_loop) : 
        fd(fd), event_loop(event_loop) {}
    TCPAcceptor(const TCPAcceptor &) = delete;
    TCPAcceptor(TCPAcceptor &&other) {
        event_loop = other.event_loop;
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

    inline void when_acceptable(Handler callback, IExecutor * executor) {
        event_loop->add_handler_for_event(
            Event(fd, Event::TO_ACCEPT),
            std::move(callback),
            executor
        );
    }
};

} // namespace ft::io

#endif // FT_TCPACCEPTOR_HPP