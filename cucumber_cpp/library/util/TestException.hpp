#ifndef UTIL_TEST_EXCEPTION_HPP
#define UTIL_TEST_EXCEPTION_HPP

#include <optional>
#include <string>

namespace cucumber_cpp::library::util
{
    struct TestException
    {
        std::string type;
        std::optional<std::string> message;
    };
}

#endif
