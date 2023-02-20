#ifndef FT_KQUEUE_BASED_L_HPP
# define FT_KQUEUE_BASED_L_HPP

# ifdef USE_KQUEUE

#  include <errno.h>
#  include <sys/event.h>
#  include <unistd.h>
#  include <cstdlib>
#  include <cstring>
#  include "non_block.hpp"
#  include "event.hpp"
#  include <iostream>
#  include <stdexcept>

#  define KQUEUE_SIZE 100

namespace ft::io::detail {


class _low_level_listener {

    int _kqueue;
    struct kevent k_evs[KQUEUE_SIZE];

    static constexpr int INFINITELY = -1; 
public:
    _low_level_listener() {
        _kqueue = kqueue();
        if (_kqueue == -1) {
            // TODO
            std::cerr << "Create kqueue" << std::endl;
            std::terminate();
        }
    }

    _low_level_listener(const _low_level_listener&) = delete;
    _low_level_listener(_low_level_listener&& other) {
        _kqueue = other._kqueue;
        other._kqueue = 0;
    }

    void add_event(_event event) {
        struct kevent k_ev;
        int fd = event.fd();
        set_nonblock(fd);

        if (event.event_type() == event.TO_READ ||
            event.event_type() == event.TO_ACCEPT) {
                EV_SET(&k_ev, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
            }
        else if (event.event_type() == event.TO_WRITE) {
            EV_SET(&k_ev, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        }
        
        int e_code = kevent(_kqueue, &k_ev, 1, NULL, 0, NULL);
        
        if (e_code) {
            std::cerr << "Add event" << std::endl;
            std::terminate();
            // throw std::logic_error(strerror(errno));
        }
    }

    void del_event(_event event) {
        struct kevent k_ev;
        int fd = event.fd();
        EV_SET(&k_ev, fd, EVFILT_READ | EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        int e_code = kevent(_kqueue, &k_ev, 1, NULL, 0, NULL);
        
        if (e_code < 0) {
            // TODO
            std::cerr << "Del event" << std::endl;
            std::terminate();
        }
    }

    void block(std::vector<_event> & events) {
        int fd_cnt = kevent(_Kqueue, NULL, 0, k_evs, KQUEUE_SIZE, INFINITELY);
        if (fd_cnt == -1) {
            // TODO
            std::cerr << "block event" << std::endl;
            std::terminate();
        }

        for (int i = 0; i < fd_cnt; ++i) {
            auto &ev = k_evs[i];
            int fd = ev.ident;
            if (ev.flags & EV_EOF)
                events.push_back(_event(fd, _event::CLOSED));
            else if (ev.flags & EV_ERROR)
                events.push_back(_event(fd, _event::ERROR));
            else if (ev.filter & EVFILT_READ)
                events.push_back(_event(fd, _event::TO_READ));
            else if (ev.filter & EVFILT_WRITE)
                events.push_back(_event(fd, _event::TO_WRITE));
        }
    }


    ~_low_level_listener() {
        if (_epfd != 0)
            close(_epfd);
    }
};

} // namespace ft::io::detail


# endif // USE_KQUEUE


#endif // FT_KQUEUE_BASED_L_HPP