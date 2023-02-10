#include "EventLoop.hpp"
#include "Socket.hpp"
#include "TCPAcceptor.hpp"

namespace ft::io {

// void EventLoop::_handle_read_ev(Event ev) {
//     auto [_handler, executor] = 
//         _ev_storage.get_read_handler(ev);
    
//     executor->execute(
//         [_handler = std::move(_handler),
//         socket_fd = ev.fd()](){
//             auto sock = Socket(socket_fd);
//             Data data = std::move(sock._read_all());
//             _handler(std::move(data));
//         }
//     );
// }

// void EventLoop::_handle_write_ev(Event ev) {
//     auto [data, _handler, executor] = 
//         _ev_storage.get_write_handler(ev);
    
//     executor->execute(
//         [_handler = std::move(_handler),
//         socket_fd = ev.fd()](){
//             Data data = _handler();
//             auto sock = Socket(socket_fd);
//             sock._write_all(data);
//         }
//     );
// }

// void EventLoop::_handle_accept_ev(Event ev) {
//     auto [_handler, executor] = 
//         _ev_storage.get_accept_handler(ev);
    
//     executor->execute(
//         [_handler = std::move(_handler),
//         acceptor_sock = ev.fd()](){
//             auto acceptor = TCPAcceptor(acceptor_sock);
//             auto sock = std::move(acceptor.accept_conn());
//             _handler(std::move(sock));
//         }
//     );
// }


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
            
            // switch (ev.event_type()) {
            // case ev.TO_READ:
            //     _handle_read_ev(ev);
            //     break;
            
            // case ev.TO_WRITE:
            //     _handle_write_ev(ev);
            //     break;

            // case ev.TO_ACCEPT:
            //     _handle_accept_ev(ev);
            //     break;
            // }
        }
    }
}

} // namespace ft::io
