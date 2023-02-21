# Webserv
A project inspired by en.subject.pdf is a project from ecole 42, but written in C+\+20.
A simple web server (similar to nginx, but with minimal functionality) written in C+\+20 
with a small self-written library for asynchronous calculations, named ft-async-core
and other a small self-written library for http protocol, what uses ft-async-core.
Webserv supports cgi, proxying and sending static files.

##### The ft-async-core library includes:
- Future/Promise:
  - map
  - flatmap
  - collect 
  - fastest (like any)
  - co_await
- Executors:
    - Static Thread Pool
    - Same Thread
- TCPAcceptor
- Socket
- Pipe
- FutTCPAcceptor - wrapper over TCPAcceptor, with methods that return Future
- FutSocket - wrapper over Socket, with methods that return Future
- FutPipe - wrapper over Pipe, with methods that return Future
- mpmc-queue
- EntityConcepts:
    - EntityCreator - concept for describing a state machine 
    that receives data and builds some entity based on it
    - EntitySerializer - concept for describing a state machine
    that receives an entity and returning serialized data
- Read, Write, AsyncRead, AsyncWrite - io concepts for describing readers and writers (as well as asynchronous)
- FutRead and FutWrite - general functions that receive abstract readers and writers, 
and build with the help of EntityCreator and EntitySerializer futures from entities

##### The http-module includes:

- HttpConnection - http client absraction on server-side;
\- a wrapper over a socket, with methods for receiving a request (future) and sending a response
- HttpConnection - http client absraction on client-side
\- a wrapper over a socket, with methods for sending request and receiving a response (future)
- HttpServer - http server absraction;
\- a wrapper over a socket, with method for accepting new connection

#### Installation and running

```sh
cmake .
make webserv
./webserv webserv.conf
```

##### webserv.conf simple example

```txt
http {

    server {
        listen 8080;
        server_name localhost:8080;

        location /cgi/hello_world {
            accepted_method         GET;
            cgi cgi-templates/cgi1.py;
        }

        location /static/html/hello_world {
            accepted_method       GET;
            root static/text.html;
        }

        location /proxy/hello_world {
            accepted_method     GET;
            proxy_pass          8032;
        }
    }
}
```
