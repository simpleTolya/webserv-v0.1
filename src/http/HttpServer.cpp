#include "HttpServer.hpp"

namespace ft::http {

io::Result<Server>   Server::create(const std::string &ip, 
                                int port, io::ExecutionContext *ctx) {
    using _Result = io::Result<Server>;

    (void) ip; // TODO provide ip
    auto res = io::TCPAcceptor::local_with_port(port, ctx);
    if (res.is_err())
        return _Result(res.get_err());
    
    auto acceptor = std::move(res.get_val());
    auto fut_acceptor = io::FutTCPAcceptor(std::move(acceptor));
    return _Result(std::move(Server(fut_acceptor)));
}

Future<io::Result<Connection>> Server::get_conn() {
    using _Result = io::Result<Connection>;
    
    return acceptor.accept_conn().map([](auto res) mutable {
        if (res.is_err())
            return _Result(std::move(res.get_err()));
        auto [_socket, addr_info] = std::move(res.get_val());
        auto fut_socket = io::FutSocket(std::move(_socket));
        return _Result(std::move(Connection(fut_socket, addr_info)));
    });
}


} // namespace ft::http
