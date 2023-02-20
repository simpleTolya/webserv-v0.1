#ifndef FT_ASYNC_CORE_HPP
# define FT_ASYNC_CORE_HPP

// including all headers from minilib "ft-async-core"

# include <async-core/executors/IExecutor.hpp>
# include <async-core/executors/same_thread.hpp>
# include <async-core/executors/StaticThreadPool.hpp>

# include <async-core/coro/stackless.hpp>
# include <async-core/future/future.hpp>

# include <async-core/io/FutSocket.hpp>
# include <async-core/io/FutTCPAcceptor.hpp>
# include <async-core/io/error/Result.hpp>

# include <async-core/mpmc-queue/mpmc_queue.hpp>

# include <async-core/util/move_only_func.hpp>
# include <async-core/util/stream_wrappers.hpp>
# include <async-core/util/util.hpp>
# include <async-core/util/Result.hpp>

#endif // FT_ASYNC_CORE_HPP