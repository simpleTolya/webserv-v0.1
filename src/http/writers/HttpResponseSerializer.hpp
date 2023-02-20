#ifndef FT_HTTP_RESPONSE_SERIALIZER_HPP
# define FT_HTTP_RESPONSE_SERIALIZER_HPP

# include <vector>
# include "../../async-core/io/EntityConcepts.hpp"

namespace ft::http {

struct HttpResponse;

struct HttpResponseSerializer {
// implements concept io::EntitySerializer
private:
    enum class State {
        HEADERS,
        BODY,
        READY
    } _state = State::HEADERS;
public:
    using Entity = HttpResponse;

    std::vector<u_char> headers;
    std::vector<u_char> body;

    HttpResponseSerializer(HttpResponse);
    static HttpResponseSerializer from(HttpResponse);

    io::Data get_data();
    io::State state() const noexcept;
};

} // namespace ft::http


#endif // FT_HTTP_RESPONSE_SERIALIZER_HPP