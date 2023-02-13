#ifndef FT_HTTP_CLIENT_HPP
# define FT_HTTP_CLIENT_HPP

# include "../async-core/io/FutRead.hpp"
# include "../async-core/io/FutWrite.hpp"
# include "../async-core/io/FutSocket.hpp"
# include "../async-core/io/FutTCPAcceptor.hpp"
# include "error/Result.hpp"

# include "HttpRequest.hpp"
# include "parsers/HttpRequestParser.hpp"
# include "writers/HttpResponseWriter.hpp"
# include "HttpResponse.hpp"

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
Future<Result<Void>> send_response(std::shared_ptr<W> w, 
                        HttpResponse response, IExecutor *e) {
    using _Result = Result<Void>;

    HttpResponseWriter writer(std::move(response));
    return ft::io::fut::write_all(w, e, std::move(writer.headers)).flatmap(
        [w, e, writer=std::move(writer)](auto res){
            if (res.is_err())
                return futures::from_val(_Result(res.get_err()));
            return ft::io::fut::write_all(w, e, std::move(writer.body))
                    .map([](auto res){
                        if (res.is_err())
                            return _Result(res.get_err());
                        return _Result(Void{});
                    });
        });
}

struct HttpConnection {
    io::FutSocket sock;

    friend HttpServer;

    HttpConnection(io::FutSocket sock) : sock(sock) {}
public:
    Future<Result<HttpRequest>> get_request(IExecutor *e) {
        return http::get_request(sock.get_impl(), e);
    }
    
    Future<Result<Void>> send_response(HttpResponse response, IExecutor *e) {
        return http::send_response(sock.get_impl(), std::move(response), e);
    }
};


} // namespace ft::http


#endif // FT_HTTP_CLIENT_HPP