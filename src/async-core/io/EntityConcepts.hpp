#ifndef FT_IO_ENTITY_CONCEPTS_HPP
# define FT_IO_ENTITY_CONCEPTS_HPP

# include <vector>
# include <optional>
# include <async-core/util/Result.hpp>
# include "error/Result.hpp"

namespace ft::io {

enum class State {
    PENDING,
    READY
};

using Data = std::vector<unsigned char>;

template<typename T, typename Entity, typename Error>
concept EntityCreator =
    requires(T creator, const Data &data, ft::io::Error err) {
        requires std::same_as<Entity, typename T::Entity>;
        requires std::same_as<Error, typename T::Error>;
        { creator(data) } -> std::convertible_to<State>;
        { creator.create_entity() } -> 
                std::convertible_to<Res<Entity, Error>>;
        { Error(err) } -> std::same_as<Error>; 
    };


template<typename T, typename Entity>
concept EntitySerializer =
    requires(T serializer, Entity entity) {
        requires std::same_as<Entity, typename T::Entity>;
        { T::from(std::move(entity)) } -> std::same_as<T>;
        { serializer.get_data() } -> std::convertible_to<Data>;
        { serializer.state() } -> std::convertible_to<State>;
    };


} // namespace ft::io


#endif // FT_IO_ENTITY_CONCEPTS_HPP