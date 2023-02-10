#include "async-core/ft-async-core.hpp"
#include "http/HttpServer.hpp"
#include "http/HttpConnection.hpp"
#include "http/HttpRequest.hpp"
#include "async-core/executors/same_thread.hpp"

ft::io::Context ctx;

void async_main();

int main(int argc, char *argv[]) {

    ft::IExecutor* executor = ft::same_thread::_();
    ft::io::EventLoop event_loop;

    ctx.event_loop = &event_loop;
    ctx.executor = executor;

    async_main();

    event_loop.loop();
    return 0;
}


void async_main() {
    auto res = ft::http::HttpServer::create(8080, ctx);

    if (res.is_err()) {
        // TODO
        return;
    }

    auto server = res.get_val();
    server.get_conn().on_complete([server](auto res){
        if (res.is_err()) {
            // TODO
        }

        auto http_conn = res.get_val();
        auto request = http_conn.get_request();
        request.on_complete([http_conn](auto req){
            if (req.is_ok())
                std::cout << req.get_val().method << std::endl;
        });
    });


}

// "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";