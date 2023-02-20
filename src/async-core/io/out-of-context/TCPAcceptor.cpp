#include "TCPAcceptor.hpp"
#include <arpa/inet.h>
#include <cstring>

namespace ft::io {

Result<TCPAcceptor> TCPAcceptor::local_with_port(
                    int port, EventLoop *event_loop) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        return Result<TCPAcceptor>(from_errno(errno));
    }

    struct sockaddr_in servaddr = (const sockaddr_in){0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
        ::close(sockfd);
        return Result<TCPAcceptor>(from_errno(errno));
    }

    if (listen(sockfd, LISTEN_BACKLOG) == -1) {
        ::close(sockfd);
        return Result<TCPAcceptor>(from_errno(errno));
    }

    return Result<TCPAcceptor>(TCPAcceptor(sockfd, event_loop));
}

Result<std::pair<Socket, InAddrInfo>> TCPAcceptor::accept_conn() {
    using _Result = Result<std::pair<Socket, InAddrInfo>>;

    sockaddr_in client_addr = (const sockaddr_in){0};
    socklen_t addr_len = sizeof(client_addr);
    
    InAddrInfo addr_info;

    int conn_fd = accept(fd, (sockaddr*)&client_addr, &addr_len);
    if (conn_fd == -1) {
        return _Result(from_errno(errno));
    }
    
    std::strcpy(addr_info.ip, inet_ntoa(client_addr.sin_addr)); 
    addr_info.port = ntohs(client_addr.sin_port);

    return _Result(std::make_pair<>(Socket(conn_fd, event_loop), addr_info));
}

} // namespace ft::io
