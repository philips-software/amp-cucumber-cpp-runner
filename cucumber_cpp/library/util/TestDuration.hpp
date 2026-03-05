#ifndef UTIL_TEST_DURATION_HPP
#define UTIL_TEST_DURATION_HPP

#include <cstddef>

namespace cucumber_cpp::library::util
{
    struct TestDuration
    {
        std::size_t seconds{};
        std::size_t nanos{};
    };
}

#endif
