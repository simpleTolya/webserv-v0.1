cmake_minimum_required(VERSION 3.0)
project(webserv)

if (NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -DNDEBUG")
# Антипаттерн использования cmake. Нужно заменить на set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")

# Подавление предупреждений пока в таком виде.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

set(FT_ASYNC_CORE_SOURCES
        src/async-core/executors/IExecutor.hpp
        src/async-core/executors/same_thread.hpp
        src/async-core/executors/StaticThreadPool.hpp

        src/async-core/future/detail/_promise_container.hpp
        src/async-core/future/detail/_shared_state.hpp
        src/async-core/future/future.hpp
        
        src/async-core/io/out-of-context/low-lewel/epoll_based_l.hpp
        src/async-core/io/out-of-context/low-lewel/kqueue_based_l.hpp
        src/async-core/io/out-of-context/low-lewel/poll_based_l.hpp
        src/async-core/io/out-of-context/low-lewel/select_based_l.hpp
        src/async-core/io/out-of-context/EventListener.hpp
        src/async-core/io/out-of-context/EventLoop.hpp
        src/async-core/io/out-of-context/EventLoop.cpp
        src/async-core/io/out-of-context/Socket.hpp
        src/async-core/io/out-of-context/Socket.cpp
        src/async-core/io/out-of-context/TCPAcceptor.hpp
        src/async-core/io/out-of-context/TCPAcceptor.cpp
        src/async-core/io/error/Result.hpp
        src/async-core/io/error/Result.cpp
        src/async-core/io/Context.hpp
        src/async-core/io/FutSocket.cpp
        src/async-core/io/FutSocket.hpp
        src/async-core/io/FutTCPAcceptor.cpp
        src/async-core/io/FutTCPAcceptor.hpp

        src/async-core/mpmc-queue/mpmc_queue.hpp
        src/async-core/mpmc-queue/detail/forward_list.hpp
        
        src/async-core/util/Result.hpp
        src/async-core/util/move_only_func.hpp
        src/async-core/util/stream_wrappers.hpp
        src/async-core/util/util.hpp)

set(FT_HTTP_SOURCES
        src/http/error/Result.hpp
        src/http/error/Result.cpp
        
        src/http/HttpConnection.cpp
        src/http/HttpConnection.hpp
        src/http/HttpResponse.hpp
        src/http/HttpRequest.hpp
        src/http/HttpRequestParser.cpp
        src/http/HttpRequestParser.hpp
        src/http/HttpServer.hpp
        src/http/HttpServer.cpp)

set(SERVER_SOURCES
        src/main.cpp)

set_source_files_properties(${SERVER_SOURCES} PROPERTIES COMPILE_FLAGS "-Wall -pedantic-errors")


# Coverage testing
add_executable(webserv_cov ${SERVER_SOURCES} ${FT_ASYNC_CORE_SOURCES} ${FT_HTTP_SOURCES})
set_target_properties(webserv_cov PROPERTIES COMPILE_FLAGS "-DTESTING_ENABLED -DCOVERAGE_ENABLED -fprofile-arcs -ftest-coverage")
set_target_properties(webserv_cov PROPERTIES LINK_FLAGS "--coverage")
set_target_properties(webserv_cov PROPERTIES EXCLUDE_FROM_ALL TRUE)

# Sanitizing
add_executable(webserv_san ${SERVER_SOURCES} ${FT_ASYNC_CORE_SOURCES} ${FT_HTTP_SOURCES})
set_target_properties(webserv_san PROPERTIES COMPILE_FLAGS "-DTESTING_ENABLED -fsanitize=undefined,address")
set_target_properties(webserv_san PROPERTIES LINK_FLAGS "-fsanitize=undefined,address")
set_target_properties(webserv_san PROPERTIES EXCLUDE_FROM_ALL TRUE)

# Valgrind
add_executable(webserv_val ${SERVER_SOURCES} ${FT_ASYNC_CORE_SOURCES} ${FT_HTTP_SOURCES})
set_target_properties(webserv_val PROPERTIES COMPILE_FLAGS "-DTESTING_ENABLED")
set_target_properties(webserv_val PROPERTIES EXCLUDE_FROM_ALL TRUE)

# Testing version
add_executable(webserv_test ${SERVER_SOURCES} ${FT_ASYNC_CORE_SOURCES} ${FT_HTTP_SOURCES})
set_target_properties(webserv_test PROPERTIES COMPILE_FLAGS "-DTESTING_ENABLED -DCOVERAGE_ENABLED ${ADDRESS_SANITIZER} -fprofile-arcs -ftest-coverage")
set_target_properties(webserv_test PROPERTIES LINK_FLAGS "${ADDRESS_SANITIZER}")
set_target_properties(webserv_test PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_executable(webserv ${SERVER_SOURCES} ${FT_ASYNC_CORE_SOURCES} ${FT_HTTP_SOURCES})

set(ASYNC_CORE_CATCH_TESTS_SOURCES
        tests/unit/future/future_test.cpp
        tests/unit/mpmc-queue/queue_test.cpp
        tests/unit/mpmc-queue/forward_list.cpp
        tests/unit/Result/result.cpp
        tests/unit/io_event/io_event.cpp
        tests/unit/executors/executors.cpp

        tests/main_test.cpp
        tests/simple_test_lib.hpp

        ${FT_ASYNC_CORE_SOURCES}
        )

add_executable(catch_tests ${ASYNC_CORE_CATCH_TESTS_SOURCES})
set_target_properties(catch_tests PROPERTIES COMPILE_FLAGS 
        "-DCATCH_TESTS -I${CMAKE_CURRENT_SOURCE_DIR}/src \
        -I${CMAKE_CURRENT_SOURCE_DIR}/tests/unit         \
        -fsanitize=undefined,address")
set_target_properties(catch_tests PROPERTIES LINK_FLAGS "-fsanitize=undefined,address")



add_executable(echo_server_v1 echo-serv-v1/server.cpp ${FT_ASYNC_CORE_SOURCES})
set_target_properties(echo_server_v1 PROPERTIES COMPILE_FLAGS 
        "-DCATCH_TESTS -I${CMAKE_CURRENT_SOURCE_DIR}/src \
        -I${CMAKE_CURRENT_SOURCE_DIR}/tests/unit         \
        -fsanitize=undefined,address")
set_target_properties(echo_server_v1 PROPERTIES LINK_FLAGS "-fsanitize=undefined,address")


add_executable(echo_server_v2 echo-serv-v2/server.cpp ${FT_ASYNC_CORE_SOURCES})
set_target_properties(echo_server_v2 PROPERTIES COMPILE_FLAGS 
        "-DCATCH_TESTS -I${CMAKE_CURRENT_SOURCE_DIR}/src \
        -I${CMAKE_CURRENT_SOURCE_DIR}/tests/unit         \
        -fsanitize=undefined,address")
set_target_properties(echo_server_v2 PROPERTIES LINK_FLAGS "-fsanitize=undefined,address")