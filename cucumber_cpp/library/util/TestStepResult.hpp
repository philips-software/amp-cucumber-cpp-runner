#ifndef UTIL_TEST_STEP_RESULT_HPP
#define UTIL_TEST_STEP_RESULT_HPP

#include "cucumber_cpp/library/util/TestDuration.hpp"
#include "cucumber_cpp/library/util/TestException.hpp"
#include "cucumber_cpp/library/util/TestStepResultStatus.hpp"
#include <optional>
#include <string>

namespace cucumber_cpp::library::util
{
    struct TestStepResult
    {
        TestDuration duration{};

        std::optional<std::string> message;

        util::TestStepResultStatus status{};

        std::optional<TestException> exception;
    };
}

#endif
