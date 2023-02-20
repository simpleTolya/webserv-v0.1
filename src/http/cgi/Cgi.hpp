#ifndef FT_HTTP_CGI_HPP
# define FT_HTTP_CGI_HPP

# include <async-core/io/FutPipe.hpp>
# include <async-core/future/future.hpp>
# include <http/HttpRequest.hpp>
# include <http/HttpResponse.hpp>

namespace ft::http::cgi {

struct Error {
    std::string err;
};

class object {
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
        const std::map<std::string, std::string>& headers,
        io::ExecutionContext *);

    Future<io::Result<Void>> send_request(http::Request);
    Future<Res<http::Response, http::Error>> get_response();
};


Future<Res<http::Response, Error>> send_request(
    http::Request, const std::string &cgi_path, io::ExecutionContext*);

} // namespace ft::io::cgi

#endif // FT_HTTP_CGI_HPP