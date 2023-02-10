#include <async-core/util/Result.hpp>
#include "../../simple_test_lib.hpp"

#include <iostream>
#include <thread>

using namespace ft;

TEST_CASE(Result) {
    
    Res<int, long>  res(5);

    ASSERT_TRUE(res.is_ok());
    ASSERT_TRUE(!res.is_err());
    ASSERT_TRUE(res.get_val() == 5);
    ASSERT_EXCEPTION(res.get_err(), std::bad_variant_access);

    auto incr = [](int a){return a + 1;};
    ASSERT_TRUE(res.map(incr)
                    .map(incr)
                    .map(incr)
                    .get_val() == 8);

    auto incr2 = [](int a){return Res<int, long>(a + 1);};
    ASSERT_TRUE(res.flat_map(incr2)
                   .flat_map(incr2)
                   .flat_map(incr2)
                   .get_val() == 8);

    ASSERT_TRUE(res.ok() == std::optional(5));
    ASSERT_TRUE(res.err() == std::nullopt);
    ASSERT_TRUE(res.or_default(10) == 5);
}
