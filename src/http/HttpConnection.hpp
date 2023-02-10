#ifndef FT_HTTP_CLIENT_HPP
# define FT_HTTP_CLIENT_HPP

# include "../async-core/io/FutSocket.hpp"
# include "../async-core/io/FutTCPAcceptor.hpp"
# include "error/Result.hpp"

namespace ft::http {

struct HttpServer;
struct HttpRequest;
struct HttpResponse;

class HttpConnection {
    io::FutSocket sock;

    friend HttpServer;

    HttpConnection(io::FutSocket sock) :
        sock(sock) {}
public:
    Future<Result<HttpRequest>> get_request();
    Future<Result<Void>> send_response(HttpResponse response);
};


} // namespace ft::http



#endif // FT_HTTP_CLIENT_HPP