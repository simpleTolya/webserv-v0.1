#ifndef FT_FUT_TCPACCEPTOR_HPP
# define FT_FUT_TCPACCEPTOR_HPP

# include "out-of-context/TCPAcceptor.hpp"
# include "../future/future.hpp"
# include "Context.hpp"

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

    Future<Result<Socket>> accept_conn(IExecutor *executor);
};

} // namespace ft::io

#endif // FT_FUT_TCPACCEPTOR_HPP