#ifndef FT_HTTP_REQUEST_SERIALIZER_HPP
# define FT_HTTP_REQUEST_SERIALIZER_HPP

# include <vector>
# include "../../async-core/io/EntityConcepts.hpp"

namespace ft::http {

struct HttpRequest;

struct HttpRequestSerializer {
// implements concept io::EntitySerializer
private:
    enum class State {
        HEADERS,
        BODY,
        READY
    } _state = State::HEADERS;
public:
    using Entity = HttpRequest;

    std::vector<u_char> headers;
    std::vector<u_char> body;

    HttpRequestSerializer(HttpRequest);
    static HttpRequestSerializer from(HttpRequest);

    io::Data get_data();
    io::State state() const noexcept;
};

} // namespace ft::http


#endif // FT_HTTP_REQUEST_SERIALIZER_HPP