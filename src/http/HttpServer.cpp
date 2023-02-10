#include "HttpServer.hpp"

namespace ft::http {

io::Result<HttpServer>   HttpServer::create(int port, io::Context ctx) {
    using _Result = io::Result<HttpServer>;

    auto res = io::TCPAcceptor::local_with_port(port);
    if (res.is_err())
        return _Result(res.get_err());
    
    auto acceptor = std::move(res.get_val());
    auto fut_acceptor = ctx.mk_acceptor(std::move(acceptor));
    return _Result(std::move(HttpServer(fut_acceptor, ctx)));
}

Future<io::Result<HttpConnection>> HttpServer::get_conn() {
    using _Result = io::Result<HttpConnection>;
    
    return acceptor.accept_conn().map([ctx=this->ctx](auto res) mutable {
        if (res.is_err())
            return _Result(std::move(res.get_err()));
        auto fut_socket = ctx.mk_socket(std::move(res.get_val()));
        return _Result(std::move(HttpConnection(fut_socket)));
    });
}


} // namespace ft::http
