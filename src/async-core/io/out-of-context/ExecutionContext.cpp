#include "ExecutionContext.hpp"
#include "Socket.hpp"
#include "TCPAcceptor.hpp"
#include <async-core/future/future.hpp>

namespace ft::io {

// event loop
void ExecutionContext::loop() {
    std::vector<Event> event_buf;
    std::vector<ft::Future<ft::Void>> futures;

    while (true) {
        event_buf.clear();
        futures.clear();

        _listener.block(event_buf);

        for (auto ev : event_buf) {
            
            _listener.del_event(ev);
            auto _handler = _ev_storage.get_handler(ev);

            Future<Void> future = ft::futures::execute<Void>([
                handler=std::move(_handler),
                event=ev.event_type()
            ]() mutable {
                handler(event);
                return Void{};
            }, executor);
            futures.push_back(std::move(future));
        }

        auto all_comleted = ft::futures::collect(
                    futures.begin(), futures.end(), futures.size());
        all_comleted.block_get();
    }
}

} // namespace ft::io
