#include "HttpResponseSerializer.hpp"
#include "../HttpResponse.hpp"

namespace ft::http {

HttpResponseSerializer::HttpResponseSerializer(HttpResponse req) {
    io::Data headers;

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

HttpResponseSerializer HttpResponseSerializer::from(HttpResponse resp) {
    return HttpResponseSerializer(std::move(resp));
}

io::Data HttpResponseSerializer::get_data() {
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
}

io::State HttpResponseSerializer::state() const noexcept {
    switch (_state)
    {
    case State::HEADERS:
    case State::BODY:
        return io::State::PENDING;

    case State::READY:
        return io::State::READY;
    }
}

} // namespace ft::http
