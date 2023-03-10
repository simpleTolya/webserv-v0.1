#include "HttpRequestSerializer.hpp"
#include <http/HttpRequest.hpp>
#include <stdexcept>

namespace ft::http {

RequestSerializer::RequestSerializer(Request req) {
    std::vector<u_char> headers;

    auto back_inserter = [&headers](const char *s) {
        for (size_t i = 0; s[i] != 0; ++i)
            headers.emplace_back(static_cast<u_char>(s[i]));
    };

    back_inserter(req.method.c_str());
    back_inserter(" ");
    back_inserter(req.path.c_str());
    back_inserter(" ");
    back_inserter(req.http_version.c_str());
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

RequestSerializer RequestSerializer::from(Request resp) {
    return RequestSerializer(std::move(resp));
}

io::Data  RequestSerializer::get_data() {
    switch (_state)
    {
    case State::HEADERS: {
        _state = State::BODY;
        return std::move(headers);
    }
    
    case State::BODY: {
        _state = State::READY;
        return std::move(body);
    }

    case State::READY:
        return {};
    }
    throw std::logic_error(
        "HttpRequestSerializer::get_data(): not match State");
}

io::State RequestSerializer::state() const noexcept {
    switch (_state)
    {
    case State::HEADERS:
    case State::BODY:
        return io::State::PENDING;

    case State::READY:
        return io::State::READY;
    }
    throw std::logic_error(
        "HttpRequestSerializer::state(): not match State");
}

} // namespace ft::http
