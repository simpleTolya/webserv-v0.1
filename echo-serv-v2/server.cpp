#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <memory>
#include <async-core/ft-async-core.hpp>

using namespace ft;
using namespace io;

// Global context for Fut* objs
Context ctx;

void async_main();

int main(int argc, char *argv[]) {

    auto executor = std::move(StaticThreadPool::create_default());
    EventLoop event_loop;

    ctx.event_loop = &event_loop;
    ctx.executor = &executor;

    async_main();

    event_loop.loop();
    return 0;
}

void pipeline_acceptor(FutTCPAcceptor acceptor);
void async_main() {

    auto res = std::move(TCPAcceptor::local_with_port(8089, ctx.event_loop));
    if (res.is_err()) {
        std::cerr << "Acceptor failed" << std::endl;
        std::cerr << ft::io::error_description(res.get_err()) << std::endl;
        return ;
    }

    auto acceptor = FutTCPAcceptor(std::move(res.get_val()));
    pipeline_acceptor(acceptor);
}

void pipeline_acceptor(FutTCPAcceptor acceptor) {
    auto res = acceptor.accept_conn(ctx.executor);
    auto socket_handled_ev = res.flatmap([acceptor](Result<Socket> sock){
        pipeline_acceptor(acceptor);

        if (sock.is_err()) {
            return futures::from_val(Result<size_t>(sock.get_err()));
        }

        auto _sock = FutSocket(std::move(sock.get_val()));
        auto data = _sock.read_part(ctx.executor);
        return data.flatmap([_sock](Result<Data> d) mutable {
            if (d.is_err()) {
                return futures::from_val(Result<size_t>(d.get_err()));
            }
            return _sock.write_all(ctx.executor, std::move(d.get_val()));
        });
    });

    socket_handled_ev.on_complete([](auto _) {
        std::cout << "Final handled client" << std::endl;
    });
}
