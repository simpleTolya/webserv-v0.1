#ifndef FT_HTTP_CLIENT_HPP
# define FT_HTTP_CLIENT_HPP

# include "../async-core/io/FutRead.hpp"
# include "../async-core/io/FutWrite.hpp"
# include "../async-core/io/FutSocket.hpp"
# include "../async-core/io/FutTCPAcceptor.hpp"
# include "error/Result.hpp"

# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "parsers/HttpRequestParser.hpp"
# include "parsers/HttpResponseParser.hpp"
# include "writers/HttpResponseSerializer.hpp"
# include "writers/HttpRequestSerializer.hpp"

namespace ft::http {

struct HttpRequest;
struct HttpResponse;
struct HttpServer;

template <typename R>
    requires ft::io::Read<R, ft::io::Error> && ft::io::AsyncRead<R>
Future<Result<HttpRequest>> get_request(std::shared_ptr<R> r, IExecutor *e) {
    return ft::io::fut::read_entity(r, HttpRequestParser(), e);
}

template <typename W>
    requires ft::io::Write<W, ft::io::Error> && ft::io::AsyncWrite<W>
Future<io::Result<Void>> send_response(std::shared_ptr<W> w, 
                        HttpResponse response, IExecutor *e) {
    return ft::io::fut::write_entity(
        w, HttpResponseSerializer(std::move(response)), e)
            .map([](auto res){
                return res.map([](auto _){return Void{};});
            });
}

template <typename R>
    requires ft::io::Read<R, ft::io::Error> && ft::io::AsyncRead<R>
Future<Result<HttpResponse>> get_response(std::shared_ptr<R> r, IExecutor *e) {
    return ft::io::fut::read_entity(r, HttpResponseParser(), e);
}

template <typename W>
    requires ft::io::Write<W, ft::io::Error> && ft::io::AsyncWrite<W>
Future<io::Result<Void>> send_request(std::shared_ptr<W> w, 
                        HttpRequest request, IExecutor *e) {
    return ft::io::fut::write_entity(
        w, HttpRequestSerializer(std::move(request)), e)
            .map([](auto res){
                return res.map([](auto _){return Void{};});
            });
}

struct HttpConnection {
    io::FutSocket sock;
    io::InAddrInfo addr_info;

    friend HttpServer;

    HttpConnection(io::FutSocket sock, io::InAddrInfo addr_info) : 
        sock(sock), addr_info(addr_info) {}
public:
    const io::InAddrInfo & get_addr_info() const noexcept {
        return addr_info;
    }

    Future<Result<HttpRequest>> get_request(IExecutor *e) {
        return http::get_request(sock.get_impl(), e);
    }
    
    Future<io::Result<Void>> send_response(HttpResponse response, IExecutor *e) {
        return http::send_response(sock.get_impl(), std::move(response), e);
    }
};


struct HttpClient {
    io::FutSocket sock;

    HttpClient(io::FutSocket sock) : sock(sock) {}
public:
    static Result<HttpClient> from(const std::string &ip, uint16_t port, 
                                                    io::EventLoop *el) {
        using _Result = Result<HttpClient>;

        auto sock_res = io::Socket::conn_tcp_serv(ip.c_str(), port, el);
        if (sock_res.is_err())
            return _Result(sock_res.get_err());
        
        auto sock = std::move(sock_res.get_val());
        return _Result(HttpClient(io::FutSocket(std::move(sock))));
    }

    Future<Result<HttpResponse>> request_response(HttpRequest request, IExecutor *e) {
        using _Result = Result<HttpResponse>;

        return send_request(request, e).flatmap([conn=*this, e](auto res) mutable {
            if (res.is_err()) {
                return futures::from_val(_Result(res.get_err()));
            }
            return conn.get_response(e);
        });
    }

    Future<Result<HttpResponse>> get_response(IExecutor *e) {
        return http::get_response(sock.get_impl(), e);
    }
    
    Future<io::Result<Void>> send_request(HttpRequest request, IExecutor *e) {
        return http::send_request(sock.get_impl(), std::move(request), e);
    }
};


} // namespace ft::http


#endif // FT_HTTP_CLIENT_HPP