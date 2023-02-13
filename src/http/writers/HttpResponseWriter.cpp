#include "HttpResponseWriter.hpp"
#include "../HttpResponse.hpp"

namespace ft::http {

HttpResponseWriter::HttpResponseWriter(HttpResponse req) {
    std::vector<u_char> headers;

    auto back_inserter = [&headers](const char *s) {
        for (size_t i = 0; s[i] != 0; ++i)
            headers.emplace_back(static_cast<u_char>(s[i]));
    };
    std::string code = std::to_string(req.code.get());

    back_inserter(req.http_version.c_str());
    back_inserter(" ");
    back_inserter(code.c_str());
    back_inserter(" ");
    back_inserter(req.status.c_str());
    back_inserter("\r\n");

    for (const auto &header : req.headers) {
        back_inserter(header.first.c_str());
        back_inserter(": ");
        back_inserter(header.second.c_str());
        back_inserter("\r\n");
    }
    back_inserter("\r\n");
    
    this->headers = std::move(headers);
    this->body = std::move(req.body);
}


} // namespace ft::http
