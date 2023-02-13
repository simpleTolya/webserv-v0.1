#ifndef FT_IO_FUTPIPE_SENDER_HPP
# define FT_IO_FUTPIPE_SENDER_HPP

# include "out-of-context/Pipe.hpp"
# include "FutWrite.hpp"
# include "FutRead.hpp"

namespace ft::io {

class FutPipeSender {
    std::shared_ptr<AsyncPipeSender> impl;
public:
    FutPipeSender(AsyncPipeSender sock) {
        impl = std::make_shared<AsyncPipeSender>(std::move(sock));
    }
    
    std::shared_ptr<AsyncPipeSender> get_impl() {
        return impl;
    }

    FT_CREATE_FUTWRITE_BODY_IMPL(AsyncPipeSender, impl)
};


class FutPipeReceiver {
    std::shared_ptr<AsyncPipeReceiver> impl;
public:
    FutPipeReceiver(AsyncPipeReceiver sock) {
        impl = std::make_shared<AsyncPipeReceiver>(std::move(sock));
    }
    
    std::shared_ptr<AsyncPipeReceiver> get_impl() {
        return impl;
    }

    FT_CREATE_FUTREAD_BODY_IMPL(AsyncPipeReceiver, impl)
};


} // namespace ft::io


#endif // FT_IO_FUTPIPE_SENDER_HPP