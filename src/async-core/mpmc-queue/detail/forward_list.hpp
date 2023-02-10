#ifndef FT_FORWARD_LIST_HPP
# define FT_FORWARD_LIST_HPP

# include <mutex>
# include <condition_variable>
# include <optional>
# include <memory>
# include <utility>
# include <functional>

namespace ft {

namespace detail {

template<
    class T,
    class Allocator = std::allocator<T>
> class forward_list {

private:
    struct _Node {
        _Node   *next;
        T   value;

        template <class... Args>
        _Node(_Node *node, Args&&... args) : 
            next(node), value(std::forward<Args>(args)...) {}

        template <class... Args>
        _Node(Args&&... args) : 
            next(nullptr), value(std::forward<Args>(args)...) {}
    };

    using node_allocator_type = 
        typename std::allocator_traits<Allocator>::template rebind_alloc<_Node>;
    using traits_t = std::allocator_traits<node_allocator_type>;
    
    _Node    *_head;
    _Node    *_tail;
    node_allocator_type _alloc;

    void _delete_values() {
        // _head node is dummy
        for (_Node *curr = _head->next; curr != nullptr;) {
            _Node *next = curr->next;

            traits_t::destroy(_alloc, curr);
            traits_t::deallocate(_alloc, curr, 1);
            
            curr = next;
        }

        // only dummy node must be deallocated
        traits_t::deallocate(_alloc, _head, 1);
    }

    void _copy_values(const forward_list & other) {

        _Node *dummy = traits_t::allocate(_alloc, 1);
        _head = dummy;
        
        _Node *curr = _head;
        _Node *other_curr = other._head->next;
        while (other_curr != nullptr) {
            curr->next = traits_t::allocate(_alloc, 1);
            traits_t::construct(_alloc, curr->next, _Node(other_curr->value));

            other_curr = other_curr->next;
            curr = curr->next;
        }
        curr->next = nullptr;
    }

public:
    using ValueType = T;
    using AllocatorType = Allocator;

    forward_list() : _alloc() {
        _Node *dummy = traits_t::allocate(_alloc, 1);
        dummy->next = nullptr;
        
        _head = dummy;
        _tail = dummy;
    }

    forward_list(const forward_list & other) : _alloc(other._alloc) {
        // TODO select_on_container_copy_construction

        _copy_values(other);
    }

    forward_list(forward_list && other) : 
        _alloc(std::move(other._alloc)),
        _head(other._head), _tail(other._tail) {
            other._head = nullptr;
            other._tail = nullptr;
    }

    forward_list& operator=( const forward_list& other ) {
        _delete_values();
        _copy_values(other);
    }

    forward_list& operator=( forward_list&& other ) {
        _delete_values();

        _head = other._head;
        _tail = other._tail;
        other._head = nullptr;
        other._tail = nullptr;
    }

    ~forward_list() { _delete_values(); }

    template <class... Args>
    void push_back(Args&&... args) {
        _Node *node = traits_t::allocate(_alloc, 1);
        traits_t::construct(_alloc, node, std::forward<Args>(args)...);

        _tail->next = node;
        _tail = node;
    }

    bool is_empty() const noexcept {
        return _head == _tail;
    }

    std::optional<T> pop_front() {
        if (is_empty()) {
            return std::nullopt;
        }

        _Node *node = _head->next;
        _head->next = node->next;
        if (node->next == nullptr) {
            _tail = _head;
        }
        std::optional<T> res(std::move(node->value));

        traits_t::destroy(_alloc, node);
        traits_t::deallocate(_alloc, node, 1);
        
        return res;
    }
};

} // namespace detail

} // namespace ft

#endif // FT_FORWARD_LIST_HPP
