#include "async-core/ft-async-core.hpp"
#include "http/ft-http.hpp"

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

void server_pipeline(ft::http::HttpServer server);
void async_main() {
    auto res = ft::http::HttpServer::create(8080, ctx.event_loop);

    if (res.is_err()) {
        std::cerr << "create server" << std::endl;
        std::exit(-1);
        return;
    }

    auto server = res.get_val();
    server_pipeline(server);
}

void server_pipeline(ft::http::HttpServer server) {
    server.get_conn(ctx.executor).on_complete([server](auto res) mutable {
        server_pipeline(server);

        if (res.is_err()) {
            std::cerr << "create server" << std::endl;
            std::exit(-1);
        }

        auto http_conn = res.get_val();
        auto request = http_conn.get_request(ctx.executor);
        request.on_complete([http_conn] (auto req) mutable {
            if (req.is_err()) {
                std::cerr << "get_request" << std::endl;
                std::exit(-1);
            }

            auto cgi_response = ft::http::cgi::send_request(
                std::move(req.get_val()),
                "/home/anatoliy/projects/cpp/webserv/cgi-templates/cgi1.py",
                ctx.event_loop,
                ctx.executor);
            
            cgi_response.on_complete([http_conn](auto cgi_response) mutable {
                if (cgi_response.is_err()) {
                    std::cerr << "cgi response" << std::endl;
                    std::exit(-1);
                }

                http_conn.send_response(std::move(cgi_response.get_val()), ctx.executor)
                    .on_complete([http_conn](auto _){});
            });
        });
    });
}
