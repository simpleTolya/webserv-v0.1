#ifndef IEXECUTOR_HPP
# define IEXECUTOR_HPP

# include "../util/move_only_func.hpp"

namespace ft {

using Task = ::detail::unique_function<void()>;

class IExecutor {
public:
    virtual void execute(Task && task) = 0;
};


} // namespace ft

#endif 