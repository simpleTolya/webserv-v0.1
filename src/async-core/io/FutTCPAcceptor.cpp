#include "FutTCPAcceptor.hpp"

namespace ft::io {

Future<Result<Socket>> FutTCPAcceptor::accept_conn() {
    using _Result = Result<Socket>;

    auto [future, promise] = ft::futures::make_contract<_Result>();
    this->impl->when_acceptable([
        acceptor=*this,
        promise=std::move(promise)]
            (Event::EventType ev) mutable {
        
            if (ev == Event::CLOSED)
                promise.set(_Result(Error::CONNECTION_CLOSED));
            
            if (ev == Event::ERROR)
                promise.set(_Result(Error::UNDEFINED));

            auto res = acceptor.impl->accept_conn();

            if (res.is_err()) {
                promise.set(_Result(res.get_err()));
            }
            promise.set(_Result(std::move(res.get_val())));
    },
    ctx.event_loop,
    ctx.executor);

    return std::move(future);
}

} // namespace ft::io