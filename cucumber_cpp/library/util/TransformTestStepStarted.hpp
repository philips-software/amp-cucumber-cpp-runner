#ifndef UTIL_TRANSFORM_TEST_STEP_STARTED_HPP
#define UTIL_TRANSFORM_TEST_STEP_STARTED_HPP

#include "cucumber/messages/test_step_started.hpp"
#include "cucumber_cpp/library/util/TestStepStarted.hpp"

namespace cucumber_cpp::library::util
{
    TestStepStarted TransformTestStepStarted(const cucumber::messages::test_step_started& testStepStarted);
}

#endif
