#ifndef FT_FUT_TCPACCEPTOR_HPP
# define FT_FUT_TCPACCEPTOR_HPP

# include "out-of-context/TCPAcceptor.hpp"
# include "../future/future.hpp"
# include "Context.hpp"

namespace ft::io {

class FutTCPAcceptor {
    Context ctx;
    friend Context;

    std::shared_ptr<TCPAcceptor> impl;

    FutTCPAcceptor(TCPAcceptor acceptor, Context ctx) {
        this->impl = std::make_shared<TCPAcceptor>(std::move(acceptor));
        this->ctx = ctx;
    }
public:
    FutTCPAcceptor(const FutTCPAcceptor &other) =default;
    // : ctx(ctx), impl(other.impl) {
    // }

    Future<Result<Socket>> accept_conn();
};


template <typename ...Args>
FutTCPAcceptor   Context::mk_acceptor(Args&&... args) {
    return FutTCPAcceptor(std::forward<Args>(args)..., *this);
}


} // namespace ft::io

#endif // FT_FUT_TCPACCEPTOR_HPP