#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <memory>
#include <async-core/ft-async-core.hpp>

// Global context for Fut* objs
ft::io::ExecutionContext *ctx;

void async_main();
int main(int argc, char *argv[]) {

    auto executor = std::move(ft::StaticThreadPool::create_default());
    ft::io::ExecutionContext context(&executor);

    ctx = &context;
    async_main();

    context.loop();
    return 0;
}

void pipeline_acceptor(ft::io::FutTCPAcceptor acceptor);
void async_main() {

    auto res = std::move(ft::io::TCPAcceptor::local_with_port(8089, ctx));
    if (res.is_err()) {
        std::cerr << "Acceptor failed" << std::endl;
        std::cerr << ft::io::error_description(res.get_err()) << std::endl;
        std::exit(-1);
    }

    auto acceptor = ft::io::FutTCPAcceptor(std::move(res.get_val()));
    pipeline_acceptor(acceptor);
}

void pipeline_acceptor(ft::io::FutTCPAcceptor acceptor) {
    auto res = acceptor.accept_conn();
    auto socket_handled_ev = res.flatmap([acceptor](auto res) mutable {
        pipeline_acceptor(acceptor);

        if (res.is_err()) {
            return ft::futures::from_val(ft::io::Result<size_t>(res.get_err()));
        }
        auto [sock, addr_info] = std::move(res.get_val());

        auto _sock = ft::io::FutSocket(std::move(sock));
        auto data = _sock.read_part();
        return data.flatmap([_sock](ft::io::Result<ft::io::Data> d) mutable {
            if (d.is_err()) {
                return ft::futures::from_val(ft::io::Result<size_t>(d.get_err()));
            }
            return _sock.write_all(std::move(d.get_val()));
        });
    });

    socket_handled_ev.on_complete([](auto _) {
        std::cout << "Final handled client" << std::endl;
    });
}
