#ifndef FT_HTTP_RESPONSE_SERIALIZER_HPP
# define FT_HTTP_RESPONSE_SERIALIZER_HPP

# include <vector>
# include <async-core/io/EntityConcepts.hpp>

namespace ft::http {

struct Response;

struct ResponseSerializer {
// implements concept io::EntitySerializer
private:
    enum class State {
        HEADERS,
        BODY,
        READY
    } _state = State::HEADERS;
public:
    using Entity = Response;

    std::vector<u_char> headers;
    std::vector<u_char> body;

    ResponseSerializer(Response);
    static ResponseSerializer from(Response);

    io::Data get_data();
    io::State state() const noexcept;
};

} // namespace ft::http


#endif // FT_HTTP_RESPONSE_SERIALIZER_HPP