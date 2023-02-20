#ifndef FT_HTTP_CLIENT_HPP
# define FT_HTTP_CLIENT_HPP

# include <async-core/io/FutRead.hpp>
# include <async-core/io/FutWrite.hpp>
# include <async-core/io/FutSocket.hpp>
# include <async-core/io/FutTCPAcceptor.hpp>
# include "error/Result.hpp"

# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "parsers/HttpRequestParser.hpp"
# include "parsers/HttpResponseParser.hpp"
# include "writers/HttpResponseSerializer.hpp"
# include "writers/HttpRequestSerializer.hpp"

namespace ft::http {

struct Request;
struct Response;
struct Server;

template <typename R>
    requires ft::io::Read<R, ft::io::Error> && ft::io::AsyncRead<R>
Future<Result<Request>> get_request(std::shared_ptr<R> r) {
    return ft::io::fut::read_entity(r, RequestParser());
}

template <typename W>
    requires ft::io::Write<W, ft::io::Error> && ft::io::AsyncWrite<W>
Future<io::Result<Void>> send_response(std::shared_ptr<W> w, 
                        Response response) {
    return ft::io::fut::write_entity(
        w, ResponseSerializer(std::move(response)))
            .map([](auto res){
                return res.map([](auto _){return Void{};});
            });
}

template <typename R>
    requires ft::io::Read<R, ft::io::Error> && ft::io::AsyncRead<R>
Future<Result<Response>> get_response(std::shared_ptr<R> r) {
    return ft::io::fut::read_entity(r, ResponseParser());
}

template <typename W>
    requires ft::io::Write<W, ft::io::Error> && ft::io::AsyncWrite<W>
Future<io::Result<Void>> send_request(std::shared_ptr<W> w, 
                        Request request) {
    return ft::io::fut::write_entity(
        w, RequestSerializer(std::move(request)))
            .map([](auto res){
                return res.map([](auto _){return Void{};});
            });
}

struct Connection {
    io::FutSocket sock;
    io::InAddrInfo addr_info;

    friend Server;

    Connection(io::FutSocket sock, io::InAddrInfo addr_info) : 
        sock(sock), addr_info(addr_info) {}
public:
    const io::InAddrInfo & get_addr_info() const noexcept {
        return addr_info;
    }

    Future<Result<http::Request>> get_request() {
        return http::get_request(sock.get_impl());
    }
    
    Future<io::Result<Void>> send_response(http::Response response) {
        return http::send_response(sock.get_impl(), std::move(response));
    }
};


struct Client {
    io::FutSocket sock;

    Client(io::FutSocket sock) : sock(sock) {}
public:
    static Result<Client> from(const std::string &ip, uint16_t port, 
                                            io::ExecutionContext *ctx) {
        using _Result = Result<Client>;

        auto sock_res = io::Socket::conn_tcp_serv(ip.c_str(), port, ctx);
        if (sock_res.is_err())
            return _Result(sock_res.get_err());
        
        auto sock = std::move(sock_res.get_val());
        return _Result(Client(io::FutSocket(std::move(sock))));
    }

    Future<Result<Response>> request_response(Request request) {
        using _Result = Result<Response>;

        return send_request(request).flatmap([conn=*this](auto res) mutable {
            if (res.is_err()) {
                return futures::from_val(_Result(res.get_err()));
            }
            return conn.get_response();
        });
    }

    Future<Result<Response>> get_response() {
        return http::get_response(sock.get_impl());
    }
    
    Future<io::Result<Void>> send_request(Request request) {
        return http::send_request(sock.get_impl(), std::move(request));
    }
};


} // namespace ft::http


#endif // FT_HTTP_CLIENT_HPP