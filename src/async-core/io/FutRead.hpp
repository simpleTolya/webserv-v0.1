#ifndef FT_IO_FUTREAD_HPP
# define FT_IO_FUTREAD_HPP

# include <async-core/future/future.hpp>
# include "out-of-context/ioConcepts.hpp"
# include <async-core/util/Result.hpp>
# include "EntityConcepts.hpp"
# include <iostream>

namespace ft::io {

using Data = std::vector<unsigned char>;

namespace fut {

template <typename R>
    requires Read<R, ft::io::Error> && AsyncRead<R>
Future<Result<Data>> read_part(std::shared_ptr<R> reader) {

    using _Result = ft::io::Result<Data>;

    auto [future, promise] = ft::futures::make_contract<_Result>();
    reader->when_readable([
        reader=reader,
        promise=std::move(promise)]
            (Event::EventType ev) mutable {
        
            if (ev == Event::CLOSED) {
                promise.set(_Result(Error::CONNECTION_CLOSED));
                return;
            }
            
            if (ev == Event::ERROR) {
                promise.set(_Result(Error::UNDEFINED));
                return;
            }

            u_char *buffer_ptr = ft::buffer;
            auto res = reader->read_part(buffer_ptr, FT_BUFFER_SIZE);

            if (res.is_err()) {
                promise.set(_Result(res.get_err()));
                return;
            }
            auto byte_cnt = res.get_val();

            Data data;
            data.reserve(byte_cnt);
            for (size_t i = 0; i < byte_cnt; ++i)
                data.emplace_back(buffer_ptr[i]);
            promise.set(_Result(std::move(data)));
    });
    
    return std::move(future);
}


// read from Reader while predicate is false
template <typename R>
    requires Read<R, ft::io::Error> && AsyncRead<R>
Future<Result<Data>> read_until(
        std::shared_ptr<R> reader,
        ::detail::unique_function<bool(const Data&)> predicate,
        Data received = {})
{
    using _Result = Result<Data>;

    Future<_Result> new_part = ft::io::fut::read_part(reader);
    auto all_data = new_part.flatmap([
        reader=reader, received=std::move(received),
        predicate=std::move(predicate)
        ](_Result part) mutable {
            
            if (part.is_err())
                return futures::from_val(_Result(part.get_err()));
            
            Data _part = std::move(part.get_val());
            received.insert(received.end(), _part.begin(), _part.end());
            
            if (not predicate(received)) {
                return ft::io::fut::read_until(reader,
                    std::move(predicate), std::move(received));
            }
            return futures::from_val(_Result(std::move(received))); 
        }
    );
    return all_data;
}


template <
    typename R,
    typename Creator,
    typename Err = R::Error,
    typename Entity = Creator::Entity,
    typename Error = Creator::Error
>       requires EntityCreator<Creator, Entity, Error> && 
                Read<R, ft::io::Error> && AsyncRead<R>
Future<Res<Entity, Error>> read_entity(
    std::shared_ptr<R> reader, Creator creator) {

    using _Result = Res<Entity, Error>;

    return ft::io::fut::read_part(reader).flatmap([
        reader = reader,
        creator=std::move(creator)
        ] (auto res) mutable {

        if (res.is_err())
            return futures::from_val(_Result(Error(res.get_err())));
        
        auto st = creator(std::move(res.get_val()));
        switch (st) {
            case ft::io::State::PENDING:
                return ft::io::fut::read_entity(
                        reader, std::move(creator));
            
            case ft::io::State::READY:
                return futures::from_val(std::move(creator.create_entity()));
        };
        throw std::logic_error("FutRead::read_entity: not match ft::io::State");
    });
}

} // namespace fut
    
} // namespace ft::io


#define FT_CREATE_FUTREAD_BODY_IMPL(Reader, reader_val)                     \
public:                                                                     \
    inline Future<Result<Data>> read_part() {                               \
        return ft::io::fut::read_part(reader_val);                          \
    }                                                                       \
                                                                            \
    inline Future<Result<Data>> read_until(                                 \
        ::detail::unique_function<bool(const Data&)> predicate) {           \
           return ft::io::fut::read_until(                                  \
                            reader_val, std::move(predicate));              \
    }                                                                       \
                                                                            \
    template <                                                              \
        typename EntityCreator,                                             \
        typename Entity = EntityCreator::Entity,                            \
        typename Error = EntityCreator::Error                               \
    > Future<Res<Entity, Error>> read_entity(EntityCreator creator) {       \
        return ft::io::fut::read_entity(reader_val, std::move(creator));    \
    }

#endif // FT_IO_FUTREAD_HPP