#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <memory>
#include <async-core/ft-async-core.hpp>


using namespace ft;
using namespace io;

void do_accept(std::shared_ptr<TCPAcceptor> acceptor, EventLoop* event_loop, IExecutor *executor);

// server
int main(int argc, char *argv[]) {
    IExecutor *executor = same_thread::_();
    
    EventLoop event_loop;

    auto res = std::move(TCPAcceptor::local_with_port(8089, &event_loop));
    if (res.is_err()) {
        std::cerr << "Acceptor failed" << std::endl;
        std::cerr << ft::io::error_description(res.get_err()) << std::endl;
        return 1;
    }
    auto acceptor = std::make_shared<TCPAcceptor>(std::move(res.get_val()));
    
    do_accept(acceptor, &event_loop, executor);

    event_loop.loop();
    return 0;
}

// callback hell
void do_accept(std::shared_ptr<TCPAcceptor> acceptor, IExecutor *executor) {
    acceptor->when_acceptable(
        [acceptor, executor](Event::EventType ev) {
            auto res = acceptor->accept_conn();
            if (res.is_err()) {
                std::cerr << "Accept failed" << std::endl;
                std::cerr << ft::io::error_description(res.get_err()) << std::endl;
                return;
            }

            Socket sock = std::move(res.get_val());
            auto shrd_sock = std::make_shared<Socket>(std::move(sock));
            shrd_sock->when_readable(
                [shrd_sock, executor](Event::EventType ev) mutable {
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
                        },
                        executor
                    );
                },

                executor
            );
            
            do_accept(acceptor, executor);
        },
        executor
    );
}
