#ifndef FT_IO_PIPE_HPP
# define FT_IO_PIPE_HPP

# include <unistd.h>
# include "ExecutionContext.hpp"
# include "async-core/io/error/Result.hpp"

namespace ft::io {

struct PipeReceiver;
struct PipeSender;
Result<std::pair<PipeSender, PipeReceiver>>  make_pipe();

class PipeReceiver {
// must implement Read
    int fd;

    PipeReceiver(int fd) : fd(fd) {}
    friend Result<std::pair<PipeSender, PipeReceiver>>  make_pipe();
protected:
    int get_fd() {
        return fd;
    }
public:
    using Error = ft::io::Error;

    PipeReceiver(const PipeReceiver & ) = delete;
    PipeReceiver(PipeReceiver && other) {
        fd = other.fd;
        other.fd = 0;
    }

    ~PipeReceiver() {
        close();
    }

    void close() {
        if (fd != 0) {
            ::close(fd);
            fd = 0;
        }
    }

    int freopen(int fd) {
        return dup2(this->fd, fd);
    }

    Result<size_t> read_part(unsigned char* buf, size_t buf_size) {
        auto cnt = read(fd, buf, buf_size);
        if (cnt == -1) {
            if (errno == EAGAIN)
                return Result<size_t>(0);
            return Result<size_t>(from_errno(errno));
        }
        return Result<size_t>(cnt);
    }
};

class PipeSender {
// must implement Write
    int fd;

    PipeSender(int fd) : fd(fd) {}
    friend Result<std::pair<PipeSender, PipeReceiver>> make_pipe();
protected:
    int get_fd() {
        return fd;
    }
public:
    using Error = ft::io::Error;

    PipeSender(const PipeSender & ) = delete;
    PipeSender(PipeSender && other) {
        fd = other.fd;
        other.fd = 0;
    }

    ~PipeSender() {
        close();
    }

    void close() {
        if (fd != 0) {
            ::close(fd);
            fd = 0;
        }
    }

    int freopen(int fd) {
        return dup2(this->fd, fd);
    }

    Result<size_t> write_part(const unsigned char* buf, size_t buf_size) {
        auto cnt = write(fd, buf, buf_size);
        if (cnt == -1) {
            if (errno == EAGAIN)
                return Result<size_t>(0);
            return Result<size_t>(from_errno(errno));
        }
        return Result<size_t>(cnt);
    }
};


class AsyncPipeSender : private PipeSender {
// implement Write & AsyncWrite
    ExecutionContext *context;
public:
    AsyncPipeSender(PipeSender ps, ExecutionContext *context) :
        PipeSender(std::move(ps)), context(context) {}

    using Error = PipeSender::Error;

    Result<size_t> write_part(const unsigned char* buf, size_t buf_size) {
        return PipeSender::write_part(buf, buf_size);
    }

    void when_writable(Handler callback) {
        context->add_handler_for_event(
            Event(get_fd(), Event::TO_WRITE),
            std::move(callback)
        );
    }
};


class AsyncPipeReceiver : private PipeReceiver {
// implement Read & AsyncRead
    ExecutionContext *event_loop;
public:
    AsyncPipeReceiver(PipeReceiver pp, ExecutionContext *el) :
        PipeReceiver(std::move(pp)), event_loop(el) {}

    using Error = PipeSender::Error;

    Result<size_t> read_part(unsigned char* buf, size_t buf_size) {
        return PipeReceiver::read_part(buf, buf_size);
    }

    void when_readable(Handler callback) {
        event_loop->add_handler_for_event(
            Event(get_fd(), Event::TO_READ),
            std::move(callback)
        );
    }
};


} // namespace ft::io

#endif // FT_IO_PIPE_HPP