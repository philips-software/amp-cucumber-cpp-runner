#ifndef UTIL_NESTED_TEST_CASE_RUNNER_ERROR_HPP
#define UTIL_NESTED_TEST_CASE_RUNNER_ERROR_HPP

#include "cucumber/messages/TestStepResult.hpp"
#include <cstddef>
#include <exception>
#include <string>
#include <utility>

namespace cucumber_cpp::library::util
{
    struct NestedTestCaseRunnerError : std::exception
    {
        NestedTestCaseRunnerError(std::size_t nesting, cucumber::messages::TestStepResult status, std::string text)
            : nesting{ nesting }
            , status{ std::move(status) }
            , text{ std::move(text) }
        {
        }

        std::size_t nesting;
        cucumber::messages::TestStepResult status;
        std::string text;

        [[nodiscard]] const char* what() const noexcept override
        {
            return text.c_str();
        }
    };
}

#endif
