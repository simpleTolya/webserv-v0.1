#include "Pipe.hpp"

namespace ft::io {

Result<std::pair<PipeSender, PipeReceiver>> make_pipe() {
    using _Result = Result<std::pair<PipeSender, PipeReceiver>>;

    int pipefd[2];
    if (pipe2(pipefd, O_NONBLOCK) == -1)
        return _Result(from_errno(errno));
    
    PipeSender sender(pipefd[1]);
    PipeReceiver receiver(pipefd[0]);
    return _Result(std::make_pair(std::move(sender), std::move(receiver)));
}

} // namespace ft::io