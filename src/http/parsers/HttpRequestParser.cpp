#include "HttpRequestParser.hpp"
#include <algorithm>


namespace ft::http {

io::State RequestParser::operator()
                    (const std::vector<u_char>& next_part) {
    
    switch (state) {

    case State::HEADERS_READ: {
        auto start_it = next_part.begin();
        while (true) {
            auto line_end_it = std::find(start_it, next_part.end(), '\n');

            if (line_end_it == next_part.end()) {
                not_handled_data.insert(not_handled_data.end(),
                                    start_it, next_part.end());
                return io::State::PENDING;
            }

            if (*(line_end_it - 1) == '\r')
                --line_end_it;

            std::string header_line;
            if (not not_handled_data.empty()) {
                header_line = std::string(
                    reinterpret_cast<char*>(not_handled_data.data()));
                not_handled_data.clear();
            }

            for (; start_it != line_end_it; ++start_it) {
                header_line.push_back(static_cast<char>(*start_it));
            }
            ++start_it; // skip \r
            if (start_it != next_part.end() && *start_it == '\n')
                ++start_it;

            if (header_line.empty()) {
                state = State::BODY_READ;
                return read_body(next_part, start_it - next_part.begin());
            }

            if (not first_line_handled) {
                first_line_handled = true;
                auto res = parse_first_header(header_line);
                if (res.is_err()) {
                    _err = res;
                    return io::State::READY;
                }
            } else {
                auto res = parse_header(std::move(header_line));
                if (res.is_err()) {
                    _err = Result<Void>(Error::HTTP_REQUEST_PARSE);
                    return io::State::READY;
                }

                auto [key, val] = std::move(res.get_val());
                headers[std::move(key)] = std::move(val);
            }
        }
    }

    case State::BODY_READ: {
        return read_body(next_part, 0);
    }

    } // switch case end
    throw std::logic_error("HttpRequestParser::operator(): not match State");
}


Result<std::pair<std::string, std::string>> 
        RequestParser::parse_header(const std::string& line) {
    
    using _Result = Result<std::pair<std::string, std::string>>;
    auto idx = line.find(":");
    if (idx == std::string::npos) {
        return _Result(Error::HTTP_REQUEST_PARSE);
    }
    auto key = line.substr(0, idx);
    auto val = line.substr(idx + 2);
    return _Result(std::make_pair<>(std::move(key), std::move(val)));
}


Result<Void>    RequestParser::set_content_length() {
    if (content_length != std::nullopt)
        return Result<Void>({}); 

    auto it = headers.find("Content-Length");
    if (it == headers.end()) {
        return Result<Void>(Error::HTTP_REQUEST_PARSE); // Content-Length
    }
    
    try {
        content_length = std::stoul(it->second);
    } catch (const std::exception &e) {
        return Result<Void>(Error::HTTP_REQUEST_PARSE); // Content-Length
    }
    return Result<Void>({});
}


Result<Request>   RequestParser::create_entity() {
    using _Result = Result<Request>; 

    if (_err.is_err()) {
        return _Result(_err.get_err());
    }

    Request request;
    request.body = std::move(this->body);
    request.headers = std::move(this->headers);
    request.method = std::move(this->method);
    request.path = std::move(this->path);
    request.http_version = std::move(this->http_version);
    return _Result(std::move(request));
}


io::State  RequestParser::read_body(
                const std::vector<u_char> &data, size_t from) {

    if (method == "GET")
        return io::State::READY;

    set_content_length();
    auto length = content_length.value();
    if (data.size() - from + body.size() >= length) {
        body.insert(body.end(), data.begin() + from, 
                        data.begin() + from + length - body.size());
        return io::State::READY; 
    } else {
        body.insert(body.end(), data.begin() + from, data.end());
        return io::State::PENDING;
    }
}


Result<Void>  RequestParser::parse_first_header(const std::string &s) {
    using _Result = Result<Void>;

    auto method_end = s.find(' ');
    if (method_end == std::string::npos or method_end == 0)
        return _Result(Error::HTTP_REQUEST_PARSE);
    this->method = s.substr(0, method_end);

    auto path_end = s.find_first_of(' ', method_end + 1);
    if (path_end == std::string::npos or path_end == method_end + 1)
        return _Result(Error::HTTP_REQUEST_PARSE);
    this->path = s.substr(method_end + 1, path_end - method_end - 1);

    this->http_version = s.substr(path_end + 1);
    if (this->http_version.empty())
        return _Result(Error::HTTP_REQUEST_PARSE);
    
    return _Result({});
}

} // namespace ft::http
