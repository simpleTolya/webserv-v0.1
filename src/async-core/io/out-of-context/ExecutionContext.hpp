#ifndef FT_EVENT_HANDLER_HPP
# define FT_EVENT_HANDLER_HPP

# include "EventListener.hpp"
# include <async-core/executors/IExecutor.hpp>
# include <async-core/util/move_only_func.hpp>
# include <async-core/util/util.hpp>

# include <unordered_map>
# include <optional>
# include <mutex>

namespace ft::io {

class Socket;
class TCPAcceptor;

using Data    = std::vector<unsigned char>;
using Handler = ::detail::unique_function<void(Event::EventType)>;

class ExecutionContext {

    class CallbackStorage {
        using fd = int;

        std::unordered_map<fd, Handler> handlers;
    public:

        void set_handler(Event ev, Handler callback) {
            handlers[ev.fd()] = std::move(callback);
        }

        Handler get_handler(Event ev) {
            auto it = handlers.find(ev.fd());
            if (it == handlers.end())
                throw std::logic_error(
                        "CallbackStorage: Event must be here");
            
            auto res = std::move(it->second);
            handlers.erase(it);
            return res;
        }
    };

private:

    EventListener   _listener;
    CallbackStorage _ev_storage;
    IExecutor       *executor;
    std::mutex      _mutex;

public:
    ExecutionContext(IExecutor *executor) : executor(executor) {};
    ExecutionContext(const ExecutionContext &) = delete;
    ExecutionContext(ExecutionContext &&) = default;

    inline void add_handler_for_event(Event event, Handler callback) {
        std::lock_guard guard(_mutex);
        _listener.add_event(event);
        _ev_storage.set_handler(event, std::move(callback));
    }

    void loop();
};

} // namespace ft::io


#endif // FT_EVENT_HANDLER_HPP