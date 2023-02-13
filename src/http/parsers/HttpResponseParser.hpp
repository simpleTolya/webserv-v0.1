#ifndef FT_HTTP_RESPONSE_PARSER_HPP
# define FT_HTTP_RESPONSE_PARSER_HPP

# include <map>
# include <string>
# include <vector>
# include "../HttpResponse.hpp"
# include "../error/Result.hpp"
# include "../../async-core/util/util.hpp"
# include "../../async-core/io/EntityCreator.hpp"

namespace ft::http {

// is io::EntityCreator
class HttpResponseParser {
    // http response params
    std::string http_version;
    std::optional<HttpResponse::Status> code;
    std::string status;
    std::map<std::string, std::string> headers;
    std::vector<u_char> body;
    
    // some metadata
    bool first_line_handled = false;
    std::vector<u_char> not_handled_data;
    std::optional<size_t> content_length = std::nullopt;
    Result<Void>    _err = Result<Void>({});

    enum class State {
        HEADERS_READ,
        BODY_READ
    } state = State::HEADERS_READ;


    static Result<std::pair<
        std::string, std::string>> parse_header(const std::string& line);

    Result<Void> parse_first_header(const std::string &line);

    Result<Void>    set_content_length();

    io::State    read_body(const std::vector<u_char> &data, size_t from);
public:
    // EntityCreator definition part
    using Error  = http::Error;
    using Entity = HttpResponse;

    io::State operator()(const std::vector<u_char>& next_part);
    Result<HttpResponse> create_entity();
};


} // namespace ft::http

#endif // FT_HTTP_RESPONSE_PARSER_HPP