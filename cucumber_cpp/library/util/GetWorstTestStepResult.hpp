#ifndef UTIL_GET_WORST_TEST_STEP_RESULT_HPP
#define UTIL_GET_WORST_TEST_STEP_RESULT_HPP

#include "cucumber/messages/TestStepResult.hpp"
#include <span>

namespace cucumber_cpp::library::util
{
    cucumber::messages::TestStepResult GetWorstTestStepResult(std::span<const cucumber::messages::TestStepResult> testStepResults);
}

#endif
