#ifndef FT_FUT_SOCKET_HPP
# define FT_FUT_SOCKET_HPP

# include "out-of-context/Socket.hpp"
# include "../future/future.hpp"
# include "../util/move_only_func.hpp"
# include "Context.hpp"
# include "EntityConcepts.hpp"
# include "out-of-context/ioConcepts.hpp"
# include "FutRead.hpp"
# include "FutWrite.hpp"
# include <memory>

namespace ft::io {

class FutSocket {
    std::shared_ptr<Socket> impl;
public:
    FutSocket(Socket sock) {
        impl = std::make_shared<Socket>(std::move(sock));
    }
    
    std::shared_ptr<Socket> get_impl() {
        return impl;
    }

    FT_CREATE_FUTREAD_BODY_IMPL(Socket, impl)
    FT_CREATE_FUTWRITE_BODY_IMPL(Socket, impl)
};

} // namespace ft::io



#endif // FT_FUT_SOCKET_HPP