#ifndef FT_EPOLL_BASED_L_HPP
# define FT_EPOLL_BASED_L_HPP

# define USE_EPOLL // DELETE ME

# ifdef USE_EPOLL

#  define EPOLL_SIZE 100

#  include <iostream>
#  include <stdexcept>
#  include <string>
#  include <errno.h>
#  include <sys/socket.h>
#  include <sys/epoll.h>
#  include <unistd.h>
#  include <cstdlib>
#  include <netinet/in.h>
#  include <netdb.h>
#  include <cstring>
#  include "non_block.hpp"

namespace ft::io::detail {

struct _event {
    enum EventType {
        TO_READ   = 0,
        TO_WRITE  = 1,
        TO_ACCEPT = 2,
        CLOSED    = 3,
        ERROR     = 4
    };

    _event(uint64_t _data) : _data(_data) {}

    _event(int fd, EventType event_type) noexcept {
        _data = static_cast<uint64_t>(event_type);
        _data |= (static_cast<uint64_t>(fd) << 32);
    }

    int fd() const noexcept {
        return static_cast<int>(_data >> 32);
    }

    EventType event_type() const noexcept {
        return static_cast<EventType>(_data & right_half);
    }

    // left 32 bits for fd and other 32 bits for event_type
    uint64_t _data;

private:
    // 32 right half 1 bits
    static constexpr uint64_t right_half = (2ull << 33ull) - 1ull;
};


class _low_level_listener {

    // epoll instance file descriptor
    int _epfd;

    // for blocking and getting events
    epoll_event _events[EPOLL_SIZE];

    static constexpr int INFINITELY = -1; 
public:
    _low_level_listener() {
        _epfd = epoll_create(EPOLL_SIZE);
        if (_epfd == -1) {
            std::cerr << "Create epoll" << std::endl;
            std::terminate();
        }
    }

    _low_level_listener(const _low_level_listener&) = delete;
    _low_level_listener(_low_level_listener&& other) {
        _epfd = other._epfd;
        other._epfd = 0;
    }

    void add_event(_event event) {
        struct epoll_event e_event;
        set_nonblock(event.fd());

        if (event.event_type() == event.TO_READ ||
            event.event_type() == event.TO_ACCEPT)
                e_event.events = EPOLLIN;
        else if (event.event_type() == event.TO_WRITE) 
            e_event.events = EPOLLOUT;

        e_event.data.u64 = event._data;
        
        int e_code = epoll_ctl(
            _epfd, EPOLL_CTL_ADD, event.fd(), &e_event);
        
        if (e_code) {
            std::cerr << "Add event" << std::endl;
            std::terminate();
            // throw std::logic_error(strerror(errno));
        }
    }

    void del_event(_event event) {
        int e_code = epoll_ctl(
            _epfd, EPOLL_CTL_DEL, event.fd(), NULL);
        
        if (e_code) {
            std::cerr << "Del event" << std::endl;
            std::terminate();
        }
    }

    void block(std::vector<_event> & events) {
        int fd_cnt = epoll_wait(_epfd, _events, EPOLL_SIZE, INFINITELY);
        if (fd_cnt == -1) {
            // TODO
            std::cerr << "block event" << std::endl;
            std::terminate();
        }

        for (int i = 0; i < fd_cnt; ++i) {
            auto e_event = _events[i];
            int fd = _event(e_event.data.u64).fd();
            if (e_event.events & EPOLLIN)
                events.push_back(_event(fd, _event::TO_READ));
            else if (e_event.events & EPOLLOUT)
                events.push_back(_event(fd, _event::TO_WRITE));
            else if (e_event.events & EPOLLHUP)
                events.push_back(_event(fd, _event::CLOSED));
            else if (e_event.events & EPOLLERR)
                events.push_back(_event(fd, _event::ERROR));
        }
    }


    ~_low_level_listener() {
        if (_epfd != 0)
            close(_epfd);
    }
};


} // namespace ft::io::detail

# endif // USE_EPOLL

#endif // FT_EPOLL_BASED_L_HPP