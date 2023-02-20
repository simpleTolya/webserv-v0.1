#include "async-core/ft-async-core.hpp"
#include "http/ft-http.hpp"
#include "config-parser/Config.hpp"
#include "config-parser/serv-conf/ServConf.hpp"
#include <fstream>

ft::io::Context ctx;
void async_main(int argc, char *argv[]);

int main(int argc, char *argv[]) {

    auto pool = ft::StaticThreadPool::create_default();

    ft::IExecutor* executor = &pool; //ft::same_thread::_();
    ft::io::EventLoop event_loop;

    ctx.event_loop = &event_loop;
    ctx.executor = executor;

    async_main(argc, argv);
    
    event_loop.loop();
    return 0;
}

void server_pipeline(ft::http::HttpServer server, 
                        std::shared_ptr<std::vector<ft::Config::Server>>);
void async_main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cerr << "Not provided path to conf with arg" << std::endl;
        std::exit(1);
    }

    std::ifstream conf_file(argv[1]);

    if (!conf_file) {
        std::cerr << "Config file error" << std::endl;
        std::exit(1);
    }

    auto serv_conf = ft::ServConf::from(conf_file);
    if (serv_conf.is_err()) {
        std::cerr << ft::ServConf::err_description(serv_conf.get_err())
                << " in conf file" << std::endl;
        std::exit(1);
    }

    auto config_res = ft::Config::from(std::move(serv_conf.get_val()));
    if (config_res.is_err()) {
        std::cerr << config_res.get_err().cause << std::endl;
        std::exit(1);
    }

    auto config = std::move(config_res.get_val());
    for (auto &[_listen, serv_info] : config.servers) {
        std::string ip, port_str = _listen;
        auto pos = _listen.find(":");
        if (pos != std::string::npos) {
            auto ip = _listen.substr(0, pos);
            auto port = _listen.substr(pos + 1);
        }
        
        uint16_t port = std::atoi(port_str.c_str()); // check err
        auto serv_res = ft::http::HttpServer::create(
                                    ip, port, ctx.event_loop);

        if (serv_res.is_err()) {
            std::cerr << "Server with listen: " << _listen <<
                " not created: " << ft::io::error_description(
                                    serv_res.get_err()) << std::endl;
            std::exit(-1);
            return;
        }

        auto server = std::move(serv_res.get_val());
        server_pipeline(server,
            std::make_shared<std::vector<ft::Config::Server>>(
                                            std::move(serv_info)));
    }
}

const ft::Config::Server::Location* get_location(
                    const ft::http::HttpRequest &req,
                    std::shared_ptr<std::vector<ft::Config::Server>> info) {
    const ft::Config::Server *server_ptr = nullptr;
    auto hostname_it = req.headers.find("Host");
    if (hostname_it == req.headers.end()) {
        server_ptr = &info->at(0);
    } else {
        auto & hostname = hostname_it->second;
        for (const auto &server : *info) {
            
            for (const auto &server_name : server.server_name) {
                if (server_name == hostname) {
                    server_ptr = &server;
                    break;
                }
            }

            if (server_ptr != nullptr) {
                break;
            }
        }
    }

    if (server_ptr == nullptr) {
        return nullptr;
    }

    const ft::Config::Server::Location *loc_ptr = nullptr;
    for (const auto &loc : server_ptr->locations) {
        const auto & path = loc.first;

        if (not loc.second.accepted_methods.contains(req.method))
            continue;

        if (not req.path.starts_with(path)) {
            continue;
        }

        loc_ptr = &loc.second;
    }
    return loc_ptr;
}

ft::http::HttpResponse with_error_code(
                    ft::http::HttpResponse::Status code);

void server_pipeline(ft::http::HttpServer server, 
                std::shared_ptr<std::vector<ft::Config::Server>> info) {
    server.get_conn(ctx.executor).on_complete([server, info](auto res) mutable {
        server_pipeline(server, info);

        if (res.is_err()) {
            ft::cerr_wrapper cerr;
            cerr << "Server accept error: " <<
                ft::io::error_description(res.get_err()) << std::endl;
            return;
        }

        auto http_conn = res.get_val();
        auto request = http_conn.get_request(ctx.executor);
        request.on_complete([http_conn, server_conf=info] (auto req_res) mutable {
            
            if (req_res.is_err()) {
                ft::cerr_wrapper cerr;
                cerr << "Http Request Error" << std::endl;
                return;
            }

            auto request = std::move(req_res.get_val());
            auto loc_ptr = get_location(request, server_conf);
            if (loc_ptr == nullptr) {
                http_conn.send_response(with_error_code(
                    ft::http::HttpResponse::Status::from(500).get_val()), ctx.executor)
                    .on_complete([http_conn](auto _){});
                return;
            }

            if (not loc_ptr->root.empty()) {
                auto res = ft::http::HttpResponse::static_from(loc_ptr->root);
                if (res.is_err()) {
                    http_conn.send_response(with_error_code(
                        ft::http::HttpResponse::Status::from(500).get_val()), ctx.executor)
                        .on_complete([http_conn](auto _){});
                    return;
                }
                auto response = std::move(res.get_val());
                http_conn.send_response(response, ctx.executor)
                        .on_complete([http_conn](auto _){});
                return;
            }

            if (not loc_ptr->cgi_path.empty()) {
                auto resp_res = ft::http::cgi::send_request(request, 
                        loc_ptr->cgi_path, ctx.event_loop, ctx.executor);
                resp_res.on_complete([http_conn](auto resp_res) mutable {
                    if (resp_res.is_err()) {
                        http_conn.send_response(with_error_code(
                            ft::http::HttpResponse::Status::from(503).get_val()), ctx.executor)
                            .on_complete([http_conn](auto _){});
                        return;
                    }
                    auto response = std::move(resp_res.get_val());
                    http_conn.send_response(response, ctx.executor)
                             .on_complete([http_conn](auto _){});
                });
                return;
            }

            if (not loc_ptr->proxy_pass.empty()) {
                auto & forwarded = request.headers["X-Forwarder-For"];
                auto client_ip = http_conn.get_addr_info().ip;
                // X-Forwarded-For: <client_ip>, <proxy1>, <proxy2>
                if (forwarded.empty()) {
                    forwarded = client_ip;
                } else {
                    forwarded.append(std::string(", ") + client_ip);
                }

                std::string proxy_ip = "127.0.0.1";
                std::string proxy_port_str;
                auto pos = loc_ptr->proxy_pass.find(':');
                if (pos == std::string::npos)
                    proxy_port_str = loc_ptr->proxy_pass;
                else {
                    proxy_ip = loc_ptr->proxy_pass.substr(0, pos);
                    proxy_port_str = loc_ptr->proxy_pass.substr(pos + 1);
                }
                uint16_t proxy_port = std::atoi(proxy_port_str.c_str());

                auto proxy_res = ft::http::HttpClient::from(
                                    proxy_ip, proxy_port, ctx.event_loop);

                if (proxy_res.is_err()) {
                    http_conn.send_response(with_error_code(
                        ft::http::HttpResponse::Status::from(502).get_val()), ctx.executor)
                        .on_complete([http_conn](auto _){});
                } else {
                    auto proxy = std::move(proxy_res.get_val());
                    proxy.request_response(request, ctx.executor)
                        .on_complete([http_conn](auto res) mutable {
                            if (res.is_err()) {
                                http_conn.send_response(with_error_code(
                                    ft::http::HttpResponse::Status::from(502).get_val()),
                                        ctx.executor)
                                    .on_complete([http_conn](auto _){});
                            } else {
                                http_conn.send_response(std::move(res.get_val()), 
                                        ctx.executor)
                                    .on_complete([http_conn](auto _){});
                            }
                        });
                }
                return;
            }

            http_conn.send_response(with_error_code(
                ft::http::HttpResponse::Status::from(404).get_val()), ctx.executor)
                .on_complete([http_conn](auto _){});
        });
    });
}

ft::http::HttpResponse with_error_code(
            ft::http::HttpResponse::Status code) {
    ft::http::HttpResponse resp;
    resp.code = code;
    resp.http_version = "HTTP/1.1";
    resp.status = "BAD_REQUEST";
    return resp;
}
