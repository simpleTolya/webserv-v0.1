#ifndef FT_FUT_SOCKET_HPP
# define FT_FUT_SOCKET_HPP

# include "out-of-context/Socket.hpp"
# include "../future/future.hpp"
# include "../util/move_only_func.hpp"
# include "Context.hpp"
# include "EntityCreator.hpp"
# include <memory>

namespace ft::io {

class FutSocket {
    Context ctx;
    friend Context;

    std::shared_ptr<Socket> impl;

    FutSocket(Socket sock, Context ctx) {
        this->impl = std::make_shared<Socket>(std::move(sock));
        this->ctx = ctx;
    }


    Future<Result<Data>> __read_until(
            ::detail::unique_function<bool(const Data&)> predicate,
            Data received);

    Future<Result<size_t>> __write_all(Data, size_t from_pos);

public:
    Future<Result<Data>> read_part();
    Future<Result<size_t>> write_part(const u_char *from, size_t count);
    
    Future<Result<size_t>> write_all(Data data) {
        return __write_all(std::move(data), 0);
    }

    // read from socket while predicate is false
    Future<Result<Data>> read_until(
        ::detail::unique_function<bool(const Data&)> predicate) {
            return __read_until(std::move(predicate), {});
    }

    template <
        typename EntityCreator, 
        typename Entity = EntityCreator::Entity,
        typename Error = EntityCreator::Error
    > Future<Res<Entity, Error>> read_entity(EntityCreator creator);
};

template <typename ...Args>
FutSocket        Context::mk_socket(Args&&... args) {
    return FutSocket(std::forward<Args>(args)..., *this);
}


template <typename EntityCreator, typename Entity, typename Error>
Future<Res<Entity, Error>>  FutSocket::read_entity(EntityCreator creator) {
    using _Result = Res<Entity, Error>;

    return this->read_part().flatmap([
        sock = *this,
        creator=std::move(creator)
        ] (auto res) mutable {

        if (res.is_err())
            return futures::from_val(_Result(Error(res.get_err())));
        
        auto st = creator(std::move(res.get_val()));
        switch (st) {
            case State::PENDING:
                return sock.read_entity(std::move(creator));
            
            case State::READY:
                return futures::from_val(std::move(creator.create_entity()));
        };
        // TODO
        std::cerr << "State not matching: " << std::endl; 
        std::terminate();
    });
}


} // namespace ft::io



#endif // FT_FUT_SOCKET_HPP