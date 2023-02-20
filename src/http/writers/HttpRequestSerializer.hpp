#ifndef FT_HTTP_REQUEST_SERIALIZER_HPP
# define FT_HTTP_REQUEST_SERIALIZER_HPP

# include <vector>
# include <async-core/io/EntityConcepts.hpp>

namespace ft::http {

struct Request;

struct RequestSerializer {
// implements concept io::EntitySerializer
private:
    enum class State {
        HEADERS,
        BODY,
        READY
    } _state = State::HEADERS;
public:
    using Entity = Request;

    std::vector<u_char> headers;
    std::vector<u_char> body;

    RequestSerializer(Request);
    static RequestSerializer from(Request);

    io::Data get_data();
    io::State state() const noexcept;
};

} // namespace ft::http


#endif // FT_HTTP_REQUEST_SERIALIZER_HPP