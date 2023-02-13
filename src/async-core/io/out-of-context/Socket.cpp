#include "Socket.hpp"

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
    unsigned char tmp_buf[BUF_SIZE];
    while (true) {
        auto res = read_part(tmp_buf, BUF_SIZE);
        if (res.is_err()) {
            return Result<Void>(res.get_err());
        }

        size_t byte_cnt = res.get_val();
        if (byte_cnt == 0) {
            return Result<Void>(Void());
        }
        
        buf.reserve(buf.size() + byte_cnt);
        for (int i = 0; i < byte_cnt; ++i)
            buf.emplace_back(tmp_buf[i]);
    }
}

} // namespace ft::io
