#ifndef FT_HTTP_SERVER_HPP
# define FT_HTTP_SERVER_HPP

# include "../async-core/io/FutSocket.hpp"
# include "../async-core/io/FutTCPAcceptor.hpp"
# include "HttpConnection.hpp"

namespace ft::http {

class HttpServer {
    ft::io::FutTCPAcceptor acceptor;

    HttpServer(ft::io::FutTCPAcceptor acceptor) :
        acceptor(acceptor) {}
public:
    static io::Result<HttpServer>   create(int port, io::EventLoop *);

    Future<io::Result<HttpConnection>> get_conn(IExecutor *e);
};


} // namespace ft::http


#endif // FT_HTTP_SERVER_HPP



/*

//          request              request
//        ----------->         -----------> 
// client              webserv              CGI
//        <-----------         <----------- 


// Content-Length
// Chunked
// Body max length exceeded

// client ---> webserv
Future<HttpRequest> from_client(Socket socket, Data received = []) {
    Future<Data> new_part = socket.read_all();

    auto http_req = new_part.flat_map([socket, received](Data data){
        received += new_part;
        if (not end_of_request(&received)) {
            return from_client(socket);
        }

        return Future<HttpRequest>(std::move(received)); 
    });

    return http_req;
}


// webserv ----> cgi
Future<Void> to_cgi(Socket sock, Data not_received) {
    Future<size_t> recv_cnt = socket.write_all(not_received);

    return recv_cnt.flat_map([sock](size_t cnt) {
        if (cnt != not_received.size()) {
            return to_cgi(sock, not_received[recv_cnt:]);
        }
        return Future<Void>();
    });
} 

// webserv <---- cgi
Future<HttpResponse> get_response(Socket service, Data received = []) {
    Future<Data> new_part = socket.read_all();

    auto http_req = new_part.flat_map([socket, received](Data data){
        received += new_part;
        if (not end_of_request(&received)) {
            return get_request(socket);
        }

        return Future<HttpResponse>(std::move(received)); 
    });

    return http_req;
}

// webserv ----> client
Future<Void> to_client(Socket sock, Data not_received) {
    Future<size_t> recv_cnt = socket.write_all(not_received);

    return recv_cnt.flat_map([sock](size_t cnt) {
        if (cnt != not_received.size()) {
            return to_client(sock, not_received[recv_cnt:]);
        }
        return Future<Void>();
    });
} 




(HttpRequest2, Route) handle_in_webserv(HttpRequest request) {
    // TODO
    return //;
}


*/




/*








// На голых сокетах
main {

    Acceptor acceptor = Acceptor::new(...);

    void pipeline_acceptor(Acceptor acceptor) {

        Future<Socket> fut_sock = acceptor.get_conn();
        Future<Void> socket_handled_event = fut_sock.flat_map([acceptor](Socket sock){
            pipeline_acceptor(acceptor);

            Future<Data> request = sock.read_all();
            Future<Data> response = request.flat_map(handle_request);
            Future<Void> event = response.flat_map([sock=std::move(sock)](Data response){
                return sock.write_all(response);
            });
            return event;
        });
        socket_handled_event.on_complete([](auto _){
            std::cout << "Final handled";
        });
    }

    TCPAcceptor server_acceptor;
    pipeline_acceptor(server_acceptor);
    event_loop.run();
}



// На Http протоколе (CORE)
main {

    void handle_connection(HttpServer server) {

        Future<HttpClient> client = server.get_conn();

        Future<Void> client_handled = client.flat_map([server](HttpClient client){
            handle_connection(server);

            Future<HttpRequest> request = client.get_request();
            Future<(HttpRequest2, Route)> req_route = request.map(handle_in_webserv);
            Future<Void> cgi_all_recv_ev = req_route.flat_map(send_with_route);
            Future<HttpResponse> from_cgi = cgi_all_recv_ev.flat_map(from_cgi);
            Future<Void> client_all_recv_ev = from_cgi.flat_map(
                    [client](HttpResponse response){client.send_response(response)});
            return client_all_recv_ev;
        });

        socket_handled_event.on_complete([](auto _){
            std::cout << "Final handled";
        });
    }

    HttpServer server = HttpServer::new(...);
    handle_connection(server);
    event_loop.run();
}


*/