#ifndef FT_EVENT_HANDLER_HPP
# define FT_EVENT_HANDLER_HPP

# include "EventListener.hpp"
# include "../../executors/IExecutor.hpp"
# include "../../util/move_only_func.hpp"
# include "../../util/util.hpp"

# include <unordered_map>
# include <optional>

namespace ft::io {

class Socket;
class TCPAcceptor;

using Data    = std::vector<unsigned char>;
using Handler = ::detail::unique_function<void(Event::EventType)>;

class EventLoop {

    class CallbackStorage {
        using fd = int;

        std::unordered_map<fd, 
            std::pair<Handler, IExecutor *> > handlers;
        // std::unordered_map<fd, 
        //     std::pair<Handler, IExecutor *> > read_handlers;
        // std::unordered_map<fd, 
        //     std::pair<Handler, IExecutor *> >  write_handlers;
        // std::unordered_map<fd, 
        //     std::pair<Handler, IExecutor *> > accept_handlers;
    public:

        void set_handler(Event ev, 
                Handler callback, IExecutor *executor) {
            handlers[ev.fd()] = std::make_pair<>(
                std::move(callback), executor
            );
        }

        // void set_read_handler(Event ev, 
        //         Handler callback, IExecutor *executor) {
        //     read_handlers[ev.fd()] = std::make_pair<>(callback, executor);
        // }

        // void set_write_handler(Event ev,
        //         Handler callback, IExecutor *executor) {
        //     write_handlers[ev.fd()] = std::make_pair<>(callback, executor);
        // }

        // void set_accept_handler(Event ev,
        //         Handler callback, IExecutor *executor) {
        //     accept_handlers[ev.fd()] = std::make_pair<>(callback, executor);
        // }

        std::pair<Handler, IExecutor *> get_handler(Event ev) {
            auto it = handlers.find(ev.fd());
            if (it == handlers.end())
                throw std::logic_error("CallbackStorage: Event must be here");
            
            auto res = std::move(it->second);
            handlers.erase(it);
            return res;
        }

        // std::pair<Handler, IExecutor *> get_read_handler(Event ev) {
        //     auto it = read_handlers.find(ev.fd());
        //     if (it == read_handlers.end())
        //         throw std::logic_error("CallbackStorage: Event must be here");
            
        //     auto res = std::move(it->second);
        //     read_handlers.erase(it);
        //     return std::move(res);
        // }

        // std::pair<Handler, IExecutor *> get_write_handler(Event ev) {
        //     auto it = write_handlers.find(ev.fd());
        //     if (it == write_handlers.end())
        //         throw std::logic_error("CallbackStorage: Event must be here");
            
        //     auto res = std::move(it->second);
        //     write_handlers.erase(it);
        //     return std::move(res);
        // }

        // std::pair<Handler, IExecutor *> get_accept_handler(Event ev) {
        //     auto it = accept_handlers.find(ev.fd());
        //     if (it == accept_handlers.end())
        //         throw std::logic_error("CallbackStorage: Event must be here");
            
        //     auto res = std::move(it->second);
        //     accept_handlers.erase(it);
        //     return std::move(res);
        // }
    };

private:

    EventListener _listener;
    CallbackStorage _ev_storage;

    inline void _handle_read_ev(Event ev);
    inline void _handle_write_ev(Event ev);
    inline void _handle_accept_ev(Event ev);

public:
    EventLoop() = default;
    EventLoop(const EventLoop &) = delete;
    EventLoop(EventLoop &&) = default;

    inline void add_handler_for_event(Event event,
            Handler callback, IExecutor *executor) {
        
        _listener.add_event(event);
        _ev_storage.set_handler(
                    event, std::move(callback), executor);
    }

    // inline void add_handler_for_event(Event event,
    //         Handler callback, IExecutor *executor) {
        
    //     _listener.add_event(event);
    //     _ev_storage.set_read_handler(
    //                     event, std::move(callback), executor);
    // }

    // inline void add_handler_for_event(Event event,
    //         Handler callback, IExecutor *executor) {
        
    //     _listener.add_event(event);
    //     _ev_storage.set_write_handler(
    //                     event, std::move(callback), executor);
    // }

    // inline void add_handler_for_event(Event event,
    //         Handler callback, IExecutor *executor) {
        
    //     _listener.add_event(event);
    //     _ev_storage.set_accept_handler(
    //                     event, std::move(callback), executor);
    // }

    void loop();
};

} // namespace ft::io


#endif // FT_EVENT_HANDLER_HPP