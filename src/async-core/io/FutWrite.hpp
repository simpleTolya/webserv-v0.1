#ifndef FT_IO_FUTWRITE_HPP
# define FT_IO_FUTWRITE_HPP

# include "../future/future.hpp"
# include "out-of-context/ioConcepts.hpp"
# include "../util/Result.hpp"
# include <iostream>

namespace ft::io {

using Data = std::vector<unsigned char>;

namespace fut {


template <typename W>
    requires Write<W, ft::io::Error> && AsyncWrite<W>
Future<Result<size_t>>  write_part(std::shared_ptr<W> writer, 
            const u_char *from, size_t count, IExecutor* executor) {
    using _Result = ft::io::Result<size_t>;

    auto [future, promise] = ft::futures::make_contract<_Result>();
    writer->when_writable([
        writer=writer, from, count,
        executor = executor,
        promise=std::move(promise)]
            (Event::EventType ev) mutable {
        
            if (ev == Event::CLOSED) {
                promise.set(_Result(Error::CONNECTION_CLOSED));
                return;
            }
            
            if (ev == Event::ERROR) {
                promise.set(_Result(Error::UNDEFINED));
                return;
            }

            auto res = writer->write_part(from, count);
            if (res.is_err()) {
                promise.set(_Result(res.get_err()));
                return;
            }
            auto byte_cnt = res.get_val();
            promise.set(_Result(byte_cnt));
    },
    executor
    );

    return std::move(future);
}


template <typename W>
  requires Write<W, ft::io::Error> && AsyncWrite<W>
Future<Result<size_t>>  write_all(std::shared_ptr<W> writer, 
                IExecutor *executor, Data d, size_t from_pos = 0) {
    using _Result = Result<size_t>;
    
    Future<_Result> send_cnt = ft::io::fut::write_part(
        writer, d.data() + from_pos, d.size() - from_pos, executor);

    auto final_cnt = send_cnt.flatmap([
        writer=writer, data=std::move(d), from_pos,
        executor=executor
        ](_Result send_cnt) mutable {
            
            if (send_cnt.is_err())
                return futures::from_val(_Result(send_cnt.get_err()));
            
            auto _send_cnt = send_cnt.get_val();
            if (from_pos + _send_cnt != data.size()) {
                return ft::io::fut::write_all(writer, 
                        executor, std::move(data), from_pos + _send_cnt);
            }
            return futures::from_val(_Result(data.size()));
        }
    );
    return final_cnt;
}


} // namespace fut
    
} // namespace ft::io


#define FT_CREATE_FUTWRITE_BODY_IMPL(Writer, writer_val)                \
public:                                                                 \
    inline Future<Res<size_t, Writer::Error>> write_part(               \
            const u_char *from, size_t count, IExecutor *e) {           \
        return ft::io::fut::write_part(writer_val, from, count, e);     \
    }                                                                   \
                                                                        \
    inline Future<Res<size_t, Writer::Error>> write_all(                \
        IExecutor *executor, Data d, size_t from_pos = 0) {             \
           return ft::io::fut::write_all(                               \
                        writer_val, executor, std::move(d), from_pos);  \
    }                                                               


#endif // FT_IO_FUTWRITE_HPP