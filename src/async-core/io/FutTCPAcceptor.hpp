#ifndef FT_FUT_TCPACCEPTOR_HPP
# define FT_FUT_TCPACCEPTOR_HPP

# include "out-of-context/TCPAcceptor.hpp"
# include <async-core/future/future.hpp>

namespace ft::io {

class FutTCPAcceptor {
    std::shared_ptr<TCPAcceptor> impl;

public:
    FutTCPAcceptor(TCPAcceptor acceptor) {
        impl = std::make_shared<TCPAcceptor>(std::move(acceptor));
    }
    
    std::shared_ptr<TCPAcceptor> get_impl() {
        return impl;
    }

    Future<Result<std::pair<Socket, InAddrInfo>>> accept_conn();
};

} // namespace ft::io

#endif // FT_FUT_TCPACCEPTOR_HPP