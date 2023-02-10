#include "HttpConnection.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestParser.hpp"
#include "HttpResponse.hpp"

namespace ft::http {

Future<Result<HttpRequest>> HttpConnection::get_request() {
    return sock.read_entity(HttpRequestParser());
}

Future<Result<Void>> HttpConnection::send_response(HttpResponse response) {
    // TODO
    return futures::from_val(Result<Void>({}));
}


} // ft::http