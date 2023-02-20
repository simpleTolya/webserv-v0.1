#ifndef FT_EVENT_HPP
# define FT_EVENT_HPP

# include <cstdint>

namespace ft::io::detail {

struct _event {
    enum EventType {
        TO_READ   = 0,
        TO_WRITE  = 1,
        TO_ACCEPT = 2,
        CLOSED    = 3,
        ERROR     = 4
    };

    _event(uint64_t _data) : _data(_data) {}

    _event(int fd, EventType event_type) noexcept {
        _data = static_cast<uint64_t>(event_type);
        _data |= (static_cast<uint64_t>(fd) << 32);
    }

    int fd() const noexcept {
        return static_cast<int>(_data >> 32);
    }

    EventType event_type() const noexcept {
        return static_cast<EventType>(_data & right_half);
    }

    // left 32 bits for fd and other 32 bits for event_type
    uint64_t _data;

private:
    // 32 right half 1 bits
    static constexpr uint64_t right_half = (2ull << 33ull) - 1ull;
};

} // namespace ft::io::detail


#endif // FT_EVENT_HPP