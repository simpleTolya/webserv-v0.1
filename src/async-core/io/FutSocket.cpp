# include "FutSocket.hpp"

namespace ft::io {

Future<Result<Data>>    FutSocket::__read_until(
    ::detail::unique_function<bool(const Data&)> predicate,
    Data received) 
{
    using _Result = Result<Data>;

    Future<_Result> new_part = this->read_part();
    auto all_data = new_part.flatmap([
        sock=*this, received=std::move(received),
        predicate=std::move(predicate)
        ](_Result part) mutable {
            
            if (part.is_err())
                return futures::from_val(_Result(part.get_err()));
            
            Data _part = std::move(part.get_val());
            received.insert(received.end(), _part.begin(), _part.end());
            
            if (not predicate(received)) {
                return sock.__read_until(
                    std::move(predicate), std::move(received));
            }
            return futures::from_val(_Result(std::move(received))); 
        }
    );
    return all_data;
}


Future<Result<size_t>>  FutSocket::__write_all(Data d, size_t from_pos) {
    using _Result = Result<size_t>;
    
    Future<_Result> send_cnt = this->write_part(
                            d.data() + from_pos, d.size() - from_pos);

    auto final_cnt = send_cnt.flatmap([
        sock=*this, data=std::move(d), from_pos
        ](_Result send_cnt) mutable {
            
            if (send_cnt.is_err())
                return futures::from_val(_Result(send_cnt.get_err()));
            
            auto _send_cnt = send_cnt.get_val();
            if (from_pos + _send_cnt != data.size()) {
                return sock.__write_all(std::move(data), from_pos + _send_cnt);
            }
            return futures::from_val(_Result(data.size()));
        }
    );
    return final_cnt;
}

Future<Result<Data>> FutSocket::read_part() {
    using _Result = Result<Data>;

    auto [future, promise] = ft::futures::make_contract<_Result>();
    this->impl->when_readable([
        sock=*this,
        promise=std::move(promise)]
            (Event::EventType ev) mutable {
        
            if (ev == Event::CLOSED)
                promise.set(_Result(Error::CONNECTION_CLOSED));
            
            if (ev == Event::ERROR)
                promise.set(_Result(Error::UNDEFINED));

            // A thread_local buffer would be better
            unsigned char _buf[4096];
            auto res = sock.impl->read_part(_buf, 4096);

            if (res.is_err()) {
                promise.set(_Result(res.get_err()));
            }
            auto byte_cnt = res.get_val();

            Data data;
            data.reserve(byte_cnt);
            for (int i = 0; i < byte_cnt; ++i)
                data.emplace_back(_buf[i]);
            promise.set(_Result(std::move(data)));
    },
    ctx.event_loop,
    ctx.executor);
    
    return std::move(future);
}

Future<Result<size_t>>  FutSocket::write_part(
                            const u_char *from, size_t count) {
    using _Result = Result<size_t>;

    auto [future, promise] = ft::futures::make_contract<_Result>();
    this->impl->when_writable([
        sock=*this, from, count,
        promise=std::move(promise)]
            (Event::EventType ev) mutable {
        
            if (ev == Event::CLOSED)
                promise.set(_Result(Error::CONNECTION_CLOSED));
            
            if (ev == Event::ERROR)
                promise.set(_Result(Error::UNDEFINED));

            auto res = sock.impl->write_part(from, count);
            if (res.is_err()) {
                promise.set(_Result(res.get_err()));
            }
            auto byte_cnt = res.get_val();
            promise.set(_Result(byte_cnt));
    },
    ctx.event_loop,
    ctx.executor
    );

    return std::move(future);
}

} // namespace ft::io
