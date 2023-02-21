#ifndef FT_STREAM_WRAPPER_HPP
# define FT_STREAM_WRAPPER_HPP

# include <mutex>
# include <sstream>
# include <iostream>

namespace ft {

struct cout_wrapper : public std::ostringstream {
private:
    static std::recursive_mutex lock_cout;
public:
    ~cout_wrapper() {
        std::lock_guard lock(lock_cout);
        std::cout << this->str();
    }
};

struct cerr_wrapper : public std::ostringstream {
private:
    static std::recursive_mutex lock_cerr;
public:
    ~cerr_wrapper() {
        std::lock_guard lock(lock_cerr);
        std::clog << this->str();
    }
};

} // namespace ft

#endif // FT_STREAM_WRAPPER_HPP