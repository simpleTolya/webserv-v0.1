#ifndef FT_EVENT_HANDLER_HPP
# define FT_EVENT_HANDLER_HPP

# include "EventListener.hpp"
# include "../../executors/IExecutor.hpp"
# include "../../util/move_only_func.hpp"
# include "../../util/util.hpp"

# include <unordered_map>
# include <optional>
# include <mutex>

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
    public:

        void set_handler(Event ev, 
                Handler callback, IExecutor *executor) {
            handlers[ev.fd()] = std::make_pair<>(
                std::move(callback), executor
            );
        }

        std::pair<Handler, IExecutor *> get_handler(Event ev) {
            auto it = handlers.find(ev.fd());
            if (it == handlers.end())
                throw std::logic_error("CallbackStorage: Event must be here");
            
            auto res = std::move(it->second);
            handlers.erase(it);
            return res;
        }
    };

private:

    EventListener   _listener;
    CallbackStorage _ev_storage;
    std::mutex      _mutex;

public:
    EventLoop() = default;
    EventLoop(const EventLoop &) = delete;
    EventLoop(EventLoop &&) = default;

    inline void add_handler_for_event(Event event,
            Handler callback, IExecutor *executor) {
        
        std::lock_guard guard(_mutex);
        _listener.add_event(event);
        _ev_storage.set_handler(
                    event, std::move(callback), executor);
    }

    void loop();
};

} // namespace ft::io


#endif // FT_EVENT_HANDLER_HPP