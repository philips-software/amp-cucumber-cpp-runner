#ifndef UTIL_NESTED_TEST_CASE_RUNNER_ERROR_HPP
#define UTIL_NESTED_TEST_CASE_RUNNER_ERROR_HPP

#include "cucumber/messages/test_step_result.hpp"
#include <cstddef>
#include <string>

namespace cucumber_cpp::library::util
{
    struct NestedTestCaseRunnerError
    {
        std::size_t nesting;
        cucumber::messages::test_step_result status;
        std::string text;
    };
}

#endif
