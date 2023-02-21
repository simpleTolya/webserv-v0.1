#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <memory>
#include <async-core/ft-async-core.hpp>

using namespace ft;
using namespace io;

// Global context for Fut* objs
ft::io::ExecutionContext *ctx;
void async_main(int argc, char *argv[]);
int main(int argc, char *argv[]) {

    auto pool = ft::StaticThreadPool::create_default();

    ft::IExecutor* executor = &pool; //ft::same_thread::_();
    ft::io::ExecutionContext context(executor);

    ctx = &context;
    async_main(argc, argv);
    
    context.loop();
    return 0;
}

Future<Void> pipeline_acceptor(FutTCPAcceptor acceptor);
void async_main(int argc, char *argv[]) {

    auto res = std::move(TCPAcceptor::local_with_port(8089, ctx));   
    if (res.is_err()) {
        std::cerr << "Acceptor failed" << std::endl;
        std::cerr << ft::io::error_description(res.get_err()) << std::endl;
        std::exit(-1);
    }

    auto acceptor = FutTCPAcceptor(std::move(res.get_val()));
    pipeline_acceptor(acceptor);
}

Future<Void> pipeline_acceptor(FutTCPAcceptor acceptor) {
    while (true) {

        auto res = co_await acceptor.accept_conn();
        
        if (res.is_err()) {
            std::cerr << "accept_conn failed" << std::endl;
            continue;
        }

        auto [__sock, _] = std::move(res.get_val());
        auto fut_sock = ft::io::FutSocket(std::move(__sock));
        auto data_res = co_await fut_sock.read_part();
        
        if (data_res.is_err()) {
            std::cerr << "read failed" << std::endl;
            continue;
        }

        auto data = std::move(data_res.get_val());
        auto writed_res = co_await fut_sock.write_all(std::move(data));
        if (writed_res.is_err()) {
            std::cerr << "write failed" << std::endl;
            continue;
        }
    }
}
