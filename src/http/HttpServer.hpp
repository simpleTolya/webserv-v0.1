#ifndef FT_HTTP_SERVER_HPP
# define FT_HTTP_SERVER_HPP

# include <async-core/io/FutSocket.hpp>
# include <async-core/io/FutTCPAcceptor.hpp>
# include "HttpConnection.hpp"

namespace ft::http {

class Server {
    ft::io::FutTCPAcceptor acceptor;

    Server(ft::io::FutTCPAcceptor acceptor) :
        acceptor(acceptor) {}
public:
    static io::Result<Server>   create(const std::string &ip,
                                    int port, io::ExecutionContext *);

    Future<io::Result<Connection>> get_conn();
};


} // namespace ft::http


#endif // FT_HTTP_SERVER_HPP
