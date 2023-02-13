#ifndef FT_HTTP_RESPONSE_HPP
# define FT_HTTP_RESPONSE_HPP

# include <map>
# include <string>
# include "error/Result.hpp"

namespace ft::http {

struct HttpResponse {
    std::string http_version;
    class Status {
        int _val;
        explicit Status(int val) : _val(val) {}
        friend HttpResponse;
    public:
        static Result<Status> from(int val) {
            if (val < 100 or val >= 600)
                return Result<Status>(Error::HTTP_REQUEST_PARSE); // TODO
            return Result<Status>(Status(val));
        }
        static Result<Status> from(const std::string & s) {
            return from(std::atoi(s.c_str()));
        }
        int get() const noexcept {return _val;}
    } code;
    std::string status;
    std::map<std::string, std::string> headers;
    std::vector<u_char> body;

    HttpResponse() : code(599) {}
};


} // namespace ft::http


#endif // FT_HTTP_RESPONSE_HPP