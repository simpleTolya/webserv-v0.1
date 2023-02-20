#ifndef FT_HTTP_SERVER_HPP
# define FT_HTTP_SERVER_HPP

# include "../async-core/io/FutSocket.hpp"
# include "../async-core/io/FutTCPAcceptor.hpp"
# include "HttpConnection.hpp"

namespace ft::http {

class HttpServer {
    ft::io::FutTCPAcceptor acceptor;

    HttpServer(ft::io::FutTCPAcceptor acceptor) :
        acceptor(acceptor) {}
public:
    static io::Result<HttpServer>   create(const std::string &ip,
                                        int port, io::EventLoop *);

    Future<io::Result<HttpConnection>> get_conn(IExecutor *e);
};


} // namespace ft::http


#endif // FT_HTTP_SERVER_HPP
