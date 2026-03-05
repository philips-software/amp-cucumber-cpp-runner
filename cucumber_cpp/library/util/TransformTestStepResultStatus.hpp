#ifndef UTIL_TRANSFORM_TEST_STEP_RESULT_STATUS_HPP
#define UTIL_TRANSFORM_TEST_STEP_RESULT_STATUS_HPP

#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/util/TestStepResultStatus.hpp"

namespace cucumber_cpp::library::util
{
    cucumber::messages::test_step_result_status TransformTestStepResultStatus(TestStepResultStatus status);
}

#endif
