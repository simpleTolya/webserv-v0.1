#include "Cgi.hpp"
#include "../writers/HttpRequestWriter.hpp"
#include "../parsers/HttpResponseParser.hpp"
#include <cstring>

namespace ft::http::cgi {

char ** object::env_from_headers(
            const std::map<std::string, std::string>& headers) {
    char **env = new char*[headers.size() + 1];
    size_t i = 0;
    for (const auto & item : headers) {
        env[i] = new char[item.first.size() + item.second.size() + 2];
        env[i][0] = 0;
        std::strcat(env[i], item.first.c_str());
        std::strcat(env[i], "=");
        std::strcat(env[i], item.second.c_str());
        i++;
    }
    env[i] = NULL;
    return env;
}

void object::clear_env(char **env) {
    for (size_t i = 0; env[i] != NULL; ++i)
        delete [] env[i];
    delete [] env;
}



Res<object, Error> object::from(const std::string & path, 
      const std::map<std::string, std::string>& headers, io::EventLoop *el) {
    using _Result = Res<object, Error>;
    
    // the channel over which the webserver sends requests to sgi
    auto res1 = io::make_pipe();
    if (res1.is_err())
        return _Result(Error("pipe"));
    auto [serv_sender, cgi_receiver] = std::move(res1.get_val());
    
    // the channel over which the webserver receives requests from sgi
    auto res2 = io::make_pipe();
    if (res2.is_err())
        return _Result(Error("pipe"));
    auto [cgi_sender, serv_receiver] = std::move(res2.get_val());

    int child_pid = fork();
    if (child_pid == -1)
        return _Result(Error("fork"));

    if (child_pid == 0) {
        serv_receiver.close();
        serv_sender.close();

        if (cgi_sender.freopen(1) == -1 or cgi_receiver.freopen(0) == -1) {
            cgi_sender.close();
            cgi_receiver.close();
            std::exit(-1);
        }
        
        cgi_sender.close();
        cgi_receiver.close();

        char * argv[] = {"sh", "-c", const_cast<char*>(path.c_str()), NULL};
        char **env = env_from_headers(headers);
        if (execve("/bin/sh", argv, env) == -1) {
            perror("execl");
            clear_env(env);
            std::exit(-1);
        }
    } else {
        // cgi_receiver and cgi_sender will automatically 
        // close when destructors are called
        
        return _Result(object(
            io::FutPipeReceiver(
                io::AsyncPipeReceiver(std::move(serv_receiver), el)),
            io::FutPipeSender(
                io::AsyncPipeSender(std::move(serv_sender), el))
        ));
    }
}


Future<io::Result<Void>> object::send_request(
                        HttpRequest req, IExecutor *executor) {
    using _Result = io::Result<Void>;

    HttpRequestWriter w(std::move(req));
    return to_cgi.write_all(executor, std::move(w.headers)).flatmap([
        to_cgi=&this->to_cgi, executor, body = std::move(w.body)
        ] (auto res) mutable {
            if (res.is_err()) {
                return futures::from_val(_Result(res.get_err()));
            }
            return to_cgi->write_all(executor, std::move(body)).map(
                [](auto res){
                    if (res.is_err())
                        return _Result(res.get_err());
                    return _Result(Void{});
                }
            );
    });
}

Future<Res<HttpResponse, http::Error>> 
                object::get_response(IExecutor *executor) {
    return from_cgi.read_entity(HttpResponseParser(), executor);
}

Future<Res<HttpResponse, Error>> send_request(
        HttpRequest req, const std::string &cgi_path, 
        io::EventLoop *el, IExecutor *executor) {
    using _Result = Res<HttpResponse, Error>;
    auto res = cgi::object::from(cgi_path, req.headers, el);
    if (res.is_err())
        return futures::from_val(_Result(res.get_err()));

    cgi::object obj = std::move(res.get_val());
    return obj.send_request(std::move(req), executor).
            flatmap([obj, executor](auto res) mutable {
                if (res.is_err())
                    return futures::from_val(_Result(Error{
                        std::string("cgi::send_request") +
                        ft::io::error_description(res.get_err())})
                    );
                return obj.get_response(executor).map(
                    [](auto res){
                        if (res.is_err())
                            return _Result(Error{"http error"});
                        return _Result(std::move(res.get_val())); 
                    }
                );
    });
}


} // namespace ft::http::cgi