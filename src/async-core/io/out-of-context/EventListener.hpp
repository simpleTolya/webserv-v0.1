#ifndef FT_EVENT_LISTENER_HPP
# define FT_EVENT_LISTENER_HPP

# include <vector>

// only one of them must be defined in the file
# include "low-lewel/kqueue_based_l.hpp"
# include "low-lewel/epoll_based_l.hpp"
# include "low-lewel/poll_based_l.hpp"
# include "low-lewel/select_based_l.hpp"


namespace ft::io {

using Event = detail::_event;

class EventListener {

    // os-dependent listener
    detail::_low_level_listener _listener;

public:
    EventListener() = default;
    EventListener(const EventListener &) = delete;
    EventListener(EventListener && ) = default;

    inline void add_event(Event event) {
        _listener.add_event(event);
    }

    inline void del_event(Event event) {
        _listener.del_event(event);
    }

    inline std::vector<Event> block() {
        std::vector<Event> events;
        _listener.block(events);
        return events;
    }
    
    inline void block(std::vector<Event> & events) {
        _listener.block(events);
    }
};

} // namespace ft::io


#endif // FT_EVENT_LISTENER_HPP