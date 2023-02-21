#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <memory>
#include <async-core/ft-async-core.hpp>


using namespace ft;
using namespace io;

void do_accept(std::shared_ptr<TCPAcceptor> acceptor);

// Global context for Fut* objs
ft::io::ExecutionContext *ctx;

void async_main(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    IExecutor *executor = same_thread::_();

    ft::io::ExecutionContext context(executor);
    ctx = &context;

    async_main(argc, argv);

    context.loop();
    return 0;
}

void async_main(int argc, char *argv[]) {
    auto res = std::move(TCPAcceptor::local_with_port(8089, ctx));
    if (res.is_err()) {
        std::cerr << "Acceptor failed" << std::endl;
        std::cerr << ft::io::error_description(res.get_err()) << std::endl;
        std::exit(-1);
    }
    auto acceptor = std::make_shared<TCPAcceptor>(std::move(res.get_val()));
    
    do_accept(acceptor);
}

void do_accept(std::shared_ptr<TCPAcceptor> acceptor) {
    acceptor->when_acceptable(
        [acceptor](Event::EventType ev) {
            auto res = acceptor->accept_conn();

            do_accept(acceptor);
            if (res.is_err()) {
                std::cerr << "Accept failed" << std::endl;
                std::cerr << ft::io::error_description(res.get_err()) << std::endl;
                return;
            }

            auto [sock, _] = std::move(res.get_val());
            auto shrd_sock = std::make_shared<Socket>(std::move(sock));
            shrd_sock->when_readable(
                [shrd_sock](Event::EventType ev) mutable {
                    if (ev != Event::TO_READ) {
                        std::cerr << " Some other" << std::endl;
                    }

                    auto res = shrd_sock->read_vec();
                    if (res.is_err()) {
                        std::cerr << "read_all failed" << std::endl;
                        std::cerr << ft::io::error_description(res.get_err()) << std::endl;
                        return;
                    }
                    Data data = std::move(res.get_val());

                    shrd_sock->when_writable(
                        [shrd_sock, bytes = std::move(data)](Event::EventType ev) {
                            auto res = shrd_sock->write_all(bytes);
                            if (res.is_err()) {
                                std::cerr << "write_all failed" << std::endl;
                                std::cerr << ft::io::error_description(res.get_err()) << std::endl;
                                return;
                            }
                            if (res.is_ok()) {
                                std::cout << "Client handled: writed byte cnt: " <<
                                    res.get_val() << std::endl;
                                return;
                            }
                        }
                    );
                }
            );
        }
    );
}
