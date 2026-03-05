#ifndef UTIL_TRANSFORM_TEST_STEP_RESULT_HPP
#define UTIL_TRANSFORM_TEST_STEP_RESULT_HPP

#include "cucumber/messages/test_step_result.hpp"
#include "cucumber_cpp/library/util/TestStepResult.hpp"

namespace cucumber_cpp::library::util
{
    cucumber::messages::test_step_result TransformTestStepResult(util::TestStepResult result);
}

#endif
