#ifndef FT_HTTP_CGI_HPP
# define FT_HTTP_CGI_HPP

# include <unistd.h>
# include "../../async-core/io/FutPipe.hpp"
# include "../../async-core/future/future.hpp"
# include "../HttpRequest.hpp"
# include "../HttpResponse.hpp"

namespace ft::http::cgi {

struct Error {
    std::string err;
};

class object {
// TODO
public:
    io::FutPipeReceiver from_cgi;
    io::FutPipeSender   to_cgi;

    object(io::FutPipeReceiver from_cgi, io::FutPipeSender to_cgi) :
        from_cgi(from_cgi), to_cgi(to_cgi) {}

    static char ** env_from_headers(
                    const std::map<std::string, std::string>& headers);
    
    static void clear_env(char **env);
public:
    static Res<object, Error> from(const std::string & path, 
        const std::map<std::string, std::string>& headers, io::EventLoop *);

    Future<io::Result<Void>> send_request(HttpRequest, IExecutor *);
    Future<Res<HttpResponse, http::Error>> get_response(IExecutor *);
};


Future<Res<HttpResponse, Error>> send_request(
    HttpRequest, const std::string &cgi_path, io::EventLoop*, IExecutor*);

} // namespace ft::io::cgi

#endif // FT_HTTP_CGI_HPP