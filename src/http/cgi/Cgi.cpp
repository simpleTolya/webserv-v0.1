#include "Cgi.hpp"
#include <http/writers/HttpRequestSerializer.hpp>
#include <http/parsers/HttpResponseParser.hpp>
#include <cstring>
#include <unistd.h>

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
      const std::map<std::string, std::string>& headers, io::ExecutionContext *ctx) {
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
        // unreachable code
        return _Result(Error(""));
    } else {
        // cgi_receiver and cgi_sender will automatically 
        // close when destructors are called
        
        return _Result(object(
            io::FutPipeReceiver(
                io::AsyncPipeReceiver(std::move(serv_receiver), ctx)),
            io::FutPipeSender(
                io::AsyncPipeSender(std::move(serv_sender), ctx))
        ));
    }
}


Future<io::Result<Void>> object::send_request(http::Request req) {
    using _Result = io::Result<Void>;

    return to_cgi.write_entity(
            http::RequestSerializer(std::move(req)))
                .map([](auto res){
                    return res.map([](auto _){return Void{};});
                });
}

Future<Res<http::Response, http::Error>> object::get_response() {
    return from_cgi.read_entity(http::ResponseParser());
}

Future<Res<http::Response, Error>> send_request(
        http::Request req, const std::string &cgi_path, 
        io::ExecutionContext *ctx) {
    using _Result = Res<http::Response, Error>;
    auto res = cgi::object::from(cgi_path, req.headers, ctx);
    if (res.is_err())
        return futures::from_val(_Result(res.get_err()));

    cgi::object obj = std::move(res.get_val());
    return obj.send_request(std::move(req)).
            flatmap([obj](auto res) mutable {
                if (res.is_err())
                    return futures::from_val(_Result(Error{
                        std::string("cgi::send_request ") +
                        ft::io::error_description(res.get_err())})
                    );
                return obj.get_response().map(
                    [](auto res){
                        if (res.is_err())
                            return _Result(Error{"http error"});
                        return _Result(std::move(res.get_val())); 
                    }
                );
    });
}


} // namespace ft::http::cgi