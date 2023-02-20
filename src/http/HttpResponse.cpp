#include "HttpResponse.hpp"
#include <fstream>
#include <vector>

namespace ft::http {

Result<HttpResponse> HttpResponse::static_from(const std::string &file_path) {
    using _Result = Result<HttpResponse>;

    std::string content_type;
    auto pos = file_path.find_last_of('.');
    if (pos != std::string::npos) {
        std::string extension = file_path.substr(pos + 1);
        if (extension == "html") {
            content_type = "text/html";
        } else if (extension == "txt") {
            content_type = "text/plain";
        } else {
            return _Result(Error::UNDEFINED); // TODO
        }
    }

    std::ifstream file(file_path, std::ios::binary);
    if (!file)
        return _Result(Error::UNDEFINED); 

    std::vector<u_char> body(std::istreambuf_iterator<char>(file), {}); 
    HttpResponse response;
    response.body = std::move(body);
    response.code = HttpResponse::Status::from(200).get_val();
    response.status = "OK";
    response.http_version = "HTTP/1.1";
    response.headers["Content-Type"] = content_type;
    response.headers["Content-Length"] = std::to_string(response.body.size());
    return _Result(std::move(response));
}
    
} // namespace ft::http
