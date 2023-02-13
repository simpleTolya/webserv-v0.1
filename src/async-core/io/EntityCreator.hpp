#ifndef FT_IO_ENTITY_CREATOR_HPP
# define FT_IO_ENTITY_CREATOR_HPP

# include <vector>
# include <optional>
# include "../util/Result.hpp"
# include "error/Result.hpp"

namespace ft::io {

enum class State {
    PENDING,
    READY
};


template<typename T, typename Entity, typename Error>
concept EntityCreator =
    requires(T creator, const std::vector<u_char> &data, ft::io::Error err) {
        requires std::same_as<Entity, typename T::Entity>;
        requires std::same_as<Error, typename T::Error>;
        { creator(data) } -> std::convertible_to<State>;
        { creator.create_entity() } -> 
                std::convertible_to<Res<Entity, Error>>;
        { Error(err) } -> std::same_as<Error>; 
    };


} // namespace ft::io


#endif // FT_IO_ENTITY_CREATOR_HPP