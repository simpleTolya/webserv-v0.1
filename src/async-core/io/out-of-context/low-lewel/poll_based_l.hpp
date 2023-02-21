#ifndef FT_POLL_BASED_L_HPP
# define FT_POLL_BASED_L_HPP


# ifdef USE_POLL

#  include <sys/poll.h>
#  include "event.hpp"
#  include "non_block.hpp"
#  include <vector>
#  include <unordered_map>

namespace ft::io::detail {

class _low_level_listener {

    std::unordered_map<int, struct pollfd> _pollfds;

    static constexpr int INFINITELY = -1;
public:
    _low_level_listener() {
    }

    _low_level_listener(const _low_level_listener&) = delete;
    _low_level_listener(_low_level_listener&& other) {
    }

    void add_event(_event event) {
        int fd = event.fd();
        set_nonblock(fd);

        if (event.event_type() == event.TO_READ ||
                event.event_type() == event.TO_ACCEPT) {
            _pollfds[fd] = pollfd{fd, POLLIN, 0};
        }
        else if (event.event_type() == event.TO_WRITE) {
            _pollfds[fd] = pollfd{fd, POLLOUT, 0};
        }
    }

    void del_event(_event event) {
        int fd = event.fd();
        _pollfds.erase(fd);
    }

    void block(std::vector<_event> & events) {
        std::vector<struct pollfd> poll_evs(_pollfds.size());
        for (const auto &[fd, poll_fd] : _pollfds) {
            poll_evs.push_back(poll_fd);
        }

        int fd_cnt = poll(poll_evs.data(), poll_evs.size(), INFINITELY);
        if (fd_cnt == -1) {
            throw std::logic_error(
                std::string("poll block: ") + strerror(errno));
        }

        for (const auto & poll_ev : poll_evs) {
            if (poll_ev.revents & POLLIN)
                events.push_back(_event(poll_ev.fd, _event::TO_READ));
            else if (poll_ev.revents & POLLOUT)
                events.push_back(_event(poll_ev.fd, _event::TO_WRITE));
            else if (poll_ev.revents & POLLHUP)
                events.push_back(_event(poll_ev.fd, _event::CLOSED));
            if (poll_ev.revents & POLLERR)
                events.push_back(_event(poll_ev.fd, _event::ERROR));
        }
    }


    ~_low_level_listener() {
    }
};

} // namespace ft::io::detail

# endif // USE_POLL


#endif // FT_POLL_BASED_L_HPP