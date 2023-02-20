#include "EventLoop.hpp"
#include "Socket.hpp"
#include "TCPAcceptor.hpp"

namespace ft::io {

void EventLoop::loop() {
    std::vector<Event> event_buf;
    while (true) {
        event_buf.clear();

        _listener.block(event_buf);

        for (auto ev : event_buf) {
            
            _listener.del_event(ev);
            auto [_handler, executor] = _ev_storage.get_handler(ev);

            executor->execute([
                handler=std::move(_handler),
                event=ev.event_type()](){
                    handler(event);
            });
            
        }
    }
}

} // namespace ft::io
