#ifndef FT_HTTP_REQUEST_WRITER_HPP
# define FT_HTTP_REQUEST_WRITER_HPP

# include <vector>

namespace ft::http {

struct HttpResponse;

struct HttpResponseWriter {
    std::vector<u_char> headers;
    std::vector<u_char> body;

    HttpResponseWriter(HttpResponse);
};

} // namespace ft::http


#endif // FT_HTTP_REQUEST_WRITER_HPP