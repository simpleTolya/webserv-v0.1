#ifndef FT_SELECT_BASED_L_HPP
# define FT_SELECT_BASED_L_HPP

# ifdef USE_SELECT

#  include "non_block.hpp"
#  include <sys/select.h>
#  include "event.hpp"
#  include "non_block.hpp"

namespace ft::io::detail {

class _low_level_listener {

    int max_fd;

    fd_set read_fds;
	fd_set write_fds;
    fd_set err_fds;

    static constexpr timeval* INFINITELY = nullptr;
public:
    _low_level_listener() : max_fd(0) {
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_ZERO(&err_fds);
    }

    _low_level_listener(const _low_level_listener&) = delete;
    _low_level_listener(_low_level_listener&& other) {
    }

    void add_event(_event event) {
        int fd = event.fd();
        if (fd > max_fd)
            max_fd = fd;
        set_nonblock(fd);

        if (event.event_type() == event.TO_READ ||
                event.event_type() == event.TO_ACCEPT) {
            FD_SET(fd, &read_fds);
        }
        else if (event.event_type() == event.TO_WRITE) {
            FD_SET(fd, &write_fds);
        }
    }

    void del_event(_event event) {
        int fd = event.fd();
        FD_CLR(fd, &read_fds);
        FD_CLR(fd, &write_fds);
    }

    void block(std::vector<_event> & events) {
        int fd_cnt = select(max_fd + 1, &read_fds, &write_fds, 
                                            &err_fds, INFINITELY);
        if (fd_cnt == -1) {
           // TODO
        }

        for (int i = 0; i < max_fd && fd_cnt > 0; ++i) {
            if (FD_ISSET(i, &read_fds)) {
                if (read(i, 0, 0) == 0) {
                    events.emplace_back(i, _event::TO_READ);
                } else {
                    events.emplace_back(i, _event::CLOSED);
                }
            }
            if (FD_ISSET(i, &write_fds)) {
                if (write(i, 0, 0) == 0) {
                    events.emplace_back(i, _event::TO_WRITE);
                } else {
                    events.emplace_back(i, _event::CLOSED);
                }
            }
            if (FD_ISSET(i, &err_fds)) {
                events.emplace_back(i, _event::ERROR);
            }
        }
    }


    ~_low_level_listener() {
    }
};


} // namespace ft::io::detail

# endif // USE_SELECT

#endif // FT_SELECT_BASED_L_HPP