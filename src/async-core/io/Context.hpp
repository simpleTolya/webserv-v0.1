#ifndef FT_IO_CONTEXT_HPP
# define FT_IO_CONTEXT_HPP

# include "out-of-context/EventLoop.hpp"
# include "../executors/IExecutor.hpp"

namespace ft::io {

struct FutSocket;
struct FutTCPAcceptor;

// Usually one global context is used for the entire program
struct Context {

    EventLoop *event_loop = nullptr;
    IExecutor *executor = nullptr;

    template <typename ...Args>
    FutSocket        mk_socket(Args&&... args);

    template <typename ...Args>
    FutTCPAcceptor   mk_acceptor(Args&&... args);
};



} // namespace ft::io


#endif // FT_IO_CONTEXT_HPP