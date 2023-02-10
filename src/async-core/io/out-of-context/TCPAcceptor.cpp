#include "TCPAcceptor.hpp"

namespace ft::io {

Result<TCPAcceptor> TCPAcceptor::local_with_port(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        return Result<TCPAcceptor>(from_errno(errno));
    }

    struct sockaddr_in servaddr = (const sockaddr_in){0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
        close(sockfd);
        return Result<TCPAcceptor>(from_errno(errno));
    }

    if (listen(sockfd, LISTEN_BACKLOG) == -1) {
        close(sockfd);
        return Result<TCPAcceptor>(from_errno(errno));
    }

    return Result<TCPAcceptor>(TCPAcceptor(sockfd));
}

} // namespace ft::io
