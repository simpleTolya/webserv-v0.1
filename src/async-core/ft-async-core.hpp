#ifndef FT_ASYNC_CORE_HPP
# define FT_ASYNC_CORE_HPP

// including all headers from minilib "ft-async-core"

# include "executors/IExecutor.hpp"
# include "executors/same_thread.hpp"
# include "executors/StaticThreadPool.hpp"

# include "future/future.hpp"

# include "io/Context.hpp"
# include "io/FutSocket.hpp"
# include "io/FutTCPAcceptor.hpp"
# include "io/error/Result.hpp"

# include "mpmc-queue/mpmc_queue.hpp"

# include "util/move_only_func.hpp"
# include "util/stream_wrappers.hpp"
# include "util/util.hpp"
# include "util/Result.hpp"

#endif // FT_ASYNC_CORE_HPP