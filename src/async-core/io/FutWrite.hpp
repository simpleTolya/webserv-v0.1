#ifndef FT_IO_FUTWRITE_HPP
# define FT_IO_FUTWRITE_HPP

# include <async-core/future/future.hpp>
# include "out-of-context/ioConcepts.hpp"
# include "EntityConcepts.hpp"
# include <async-core/util/Result.hpp>
# include <iostream>

namespace ft::io {

using Data = std::vector<unsigned char>;

namespace fut {


template <typename W>
    requires Write<W, ft::io::Error> && AsyncWrite<W>
Future<Result<size_t>>  write_part(std::shared_ptr<W> writer,
            const u_char *from, size_t count) {
    using _Result = ft::io::Result<size_t>;

    auto [future, promise] = ft::futures::make_contract<_Result>();
    writer->when_writable([
        writer=writer, from, count,
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
    });

    return std::move(future);
}


template <typename W>
  requires Write<W, ft::io::Error> && AsyncWrite<W>
Future<Result<size_t>>  write_all(std::shared_ptr<W> writer, 
        Data d, size_t from_pos = 0, size_t sended = 0) {
    using _Result = Result<size_t>;
    
    Future<_Result> send_cnt = ft::io::fut::write_part(
        writer, d.data() + from_pos, d.size() - from_pos);

    auto final_cnt = send_cnt.flatmap([
        writer, data=std::move(d), from_pos, sended
        ](_Result send_cnt) mutable {
            
            if (send_cnt.is_err())
                return futures::from_val(_Result(send_cnt.get_err()));
            
            auto _send_cnt = send_cnt.get_val();
            sended += _send_cnt;
            if (from_pos + _send_cnt != data.size()) {
                return ft::io::fut::write_all(writer, 
                            std::move(data), from_pos + _send_cnt);
            }
            return futures::from_val(_Result(sended));
        }
    );
    return final_cnt;
}


template <
    typename W,
    typename Serializer,
    typename Entity = Serializer::Entity
> requires EntitySerializer<Serializer, Entity> &&
             Write<W, ft::io::Error> && AsyncWrite<W>
Future<Result<size_t>>  write_entity(std::shared_ptr<W> writer, 
        Serializer serializer, size_t sended = 0) {
    using _Result = Result<size_t>; 

    switch (serializer.state()) {
    case ft::io::State::PENDING:
    {
        Data data = serializer.get_data();
        return ft::io::fut::write_all(writer, std::move(data))
            .flatmap([writer, sended,serializer=std::move(serializer)
                    ] (auto res) mutable {
                
                if (res.is_err())
                    return futures::from_val(_Result(res.get_err()));
                
                return ft::io::fut::write_entity(
                    writer, std::move(serializer), sended);
            });
    }

    case ft::io::State::READY:
        return futures::from_val(_Result(sended));
    }

    throw std::logic_error(
            "FutWrite::write_entity: not matching ft::io::State");
}

} // namespace fut
    
} // namespace ft::io


#define FT_CREATE_FUTWRITE_BODY_IMPL(Writer, writer_val)                \
public:                                                                 \
    inline Future<Res<size_t, Writer::Error>> write_part(               \
            const u_char *from, size_t count) {                         \
        return ft::io::fut::write_part(writer_val, from, count);        \
    }                                                                   \
                                                                        \
    inline Future<Res<size_t, Writer::Error>> write_all(                \
        Data d, size_t from_pos = 0) {                                  \
           return ft::io::fut::write_all(                               \
                        writer_val, std::move(d), from_pos);            \
    }                                                                   \
                                                                        \
    template <                                                          \
        typename Serializer,                                            \
        typename Entity = Serializer::Entity                            \
    > Future<Res<size_t, Writer::Error>> write_entity(                  \
            Serializer serializer) {                                    \
           return ft::io::fut::write_entity(                            \
                    writer_val, std::move(serializer));                 \
    }

#endif // FT_IO_FUTWRITE_HPP