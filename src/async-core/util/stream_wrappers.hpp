#ifndef FT_STREAM_WRAPPER_HPP
# define FT_STREAM_WRAPPET_HPP

# include <mutex>
# include <sstream>
# include <iostream>

namespace ft {

// TODO сделать мьютекс статическим в классе

struct cout_wrapper : public std::ostringstream {
private:
    static std::recursive_mutex & get_lock() {
        static std::recursive_mutex lock_cout;
        return lock_cout;
    }
public:
    ~cout_wrapper() {
        std::lock_guard lock(get_lock());
        std::cout << this->str();
    }
};

struct cerr_wrapper : public std::ostringstream {
private:
    static std::recursive_mutex & get_lock() {
        static std::recursive_mutex lock_cout;
        return lock_cout;
    }
public:
    ~cerr_wrapper() {
        std::lock_guard lock(get_lock());
        std::cout << this->str();
    }
};

} // namespace ft

#endif // FT_STREAM_WRAPPET_HPP