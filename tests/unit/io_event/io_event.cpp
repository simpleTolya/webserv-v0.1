#include <async-core/io/out-of-context/EventLoop.hpp>
#include "../../simple_test_lib.hpp"

#include <iostream>
#include <thread>

using namespace ft::io;

TEST_CASE(io_event) {
    
    Event ev = Event(10, Event::TO_READ);
    ASSERT_TRUE(ev.fd() == 10)
    ASSERT_TRUE(ev.event_type() == Event::TO_READ)

    ev = Event(0, Event::CLOSED);
    ASSERT_TRUE(ev.fd() == 0)
    ASSERT_TRUE(ev.event_type() == Event::CLOSED)

    Event ev2 = Event(ev._data);
    ASSERT_TRUE(ev2.fd() == 0)
    ASSERT_TRUE(ev2.event_type() == Event::CLOSED)
}
