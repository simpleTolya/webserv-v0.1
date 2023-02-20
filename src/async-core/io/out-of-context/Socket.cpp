#include "Socket.hpp"
#include <arpa/inet.h>

namespace ft::io {

Result<size_t>  Socket::write_all(const Data &data) {
    size_t write_cnt = 0;
    while (true) {
        auto res = write_part(data.data() + write_cnt, 
                                data.size() - write_cnt);
        if (res.is_err()) {
            return res;
        }

        write_cnt += res.get_val();
        if (write_cnt == data.size()) {
            return Result<size_t>(write_cnt);
        }
    }
}


Result<Void>    Socket::read_vec(Data &buf) {
    u_char *buffer_ptr = ft::buffer;
    while (true) {
        auto res = read_part(buffer_ptr, FT_BUFFER_SIZE);
        if (res.is_err()) {
            return Result<Void>(res.get_err());
        }

        size_t byte_cnt = res.get_val();
        if (byte_cnt == 0) {
            return Result<Void>(Void());
        }
        
        buf.reserve(buf.size() + byte_cnt);
        for (int i = 0; i < byte_cnt; ++i)
            buf.emplace_back(buffer_ptr[i]);
    }
}

Result<Socket>  Socket::conn_tcp_serv(const char *ip, 
                    uint16_t port, ExecutionContext *ctx) {
    using _Result = Result<Socket>;

    sockaddr_in servaddr = (const sockaddr_in){0};
 
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        return _Result(from_errno(errno));
 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    if (connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)) != 0)
        return _Result(from_errno(errno));
    
    return _Result(Socket(sockfd, ctx));
}

} // namespace ft::io
