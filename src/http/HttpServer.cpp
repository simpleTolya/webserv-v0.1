#include "HttpServer.hpp"

namespace ft::http {

io::Result<HttpServer>   HttpServer::create(const std::string &ip, 
                                    int port, io::EventLoop *el) {
    using _Result = io::Result<HttpServer>;

    // TODO provide ip
    auto res = io::TCPAcceptor::local_with_port(port, el);
    if (res.is_err())
        return _Result(res.get_err());
    
    auto acceptor = std::move(res.get_val());
    auto fut_acceptor = io::FutTCPAcceptor(std::move(acceptor));
    return _Result(std::move(HttpServer(fut_acceptor)));
}

Future<io::Result<HttpConnection>> HttpServer::get_conn(IExecutor *e) {
    using _Result = io::Result<HttpConnection>;
    
    return acceptor.accept_conn(e).map([](auto res) mutable {
        if (res.is_err())
            return _Result(std::move(res.get_err()));
        auto [_socket, addr_info] = std::move(res.get_val());
        auto fut_socket = io::FutSocket(std::move(_socket));
        return _Result(std::move(HttpConnection(fut_socket, addr_info)));
    });
}


} // namespace ft::http
