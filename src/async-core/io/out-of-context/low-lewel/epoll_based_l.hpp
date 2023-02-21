#ifndef FT_EPOLL_BASED_L_HPP
# define FT_EPOLL_BASED_L_HPP

# define USE_EPOLL

# ifdef USE_EPOLL

#  define EPOLL_SIZE 100

#  include <errno.h>
#  include <sys/epoll.h>
#  include <unistd.h>
#  include <cstdlib>
#  include <cstring>
#  include "non_block.hpp"
#  include "event.hpp"
#  include <iostream>
#  include <stdexcept>

namespace ft::io::detail {

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
            throw std::logic_error(
                std::string("Epoll Create: ") + strerror(errno));
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
            throw std::logic_error(
                std::string("Epoll Add event: ") + strerror(errno));
        }
    }

    void del_event(_event event) {
        int e_code = epoll_ctl(
            _epfd, EPOLL_CTL_DEL, event.fd(), NULL);
        
        if (e_code) {
            throw std::logic_error(
                std::string("Epoll del event: ") + strerror(errno));
        }
    }

    void block(std::vector<_event> & events) {
        int fd_cnt = epoll_wait(_epfd, _events, EPOLL_SIZE, INFINITELY);
        if (fd_cnt == -1) {
            throw std::logic_error(
                std::string("Epoll block: ") + strerror(errno));
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