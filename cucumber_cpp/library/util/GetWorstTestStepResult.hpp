#ifndef UTIL_GET_WORST_TEST_STEP_RESULT_HPP
#define UTIL_GET_WORST_TEST_STEP_RESULT_HPP

#include "cucumber/messages/test_step_result.hpp"
#include <span>

namespace cucumber_cpp::library::util
{
    cucumber::messages::test_step_result GetWorstTestStepResult(std::span<const cucumber::messages::test_step_result> testStepResults);
}

#endif
