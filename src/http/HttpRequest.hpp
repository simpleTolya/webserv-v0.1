#ifndef FT_HTTP_REQUEST_HPP
# define FT_HTTP_REQUEST_HPP

# include <map>
# include <string>
# include <vector>
# include "error/Result.hpp"

struct HttpRequestParser;

namespace ft::http {

struct HttpRequest {
    friend HttpRequestParser;

    std::string method;
    std::string path;
    std::string http_version;

    std::map<std::string, std::string> headers;
    std::vector<unsigned char> body;
};


} // namespace ft::http



#endif // FT_HTTP_REQUEST_HPP