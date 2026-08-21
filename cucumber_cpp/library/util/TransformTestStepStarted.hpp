#ifndef UTIL_TRANSFORM_TEST_STEP_STARTED_HPP
#define UTIL_TRANSFORM_TEST_STEP_STARTED_HPP

#include "cucumber/messages/TestStepStarted.hpp"
#include "cucumber_cpp/library/util/TestStepStarted.hpp"

namespace cucumber_cpp::library::util
{
    TestStepStarted TransformTestStepStarted(const cucumber::messages::TestStepStarted& testStepStarted);
}

#endif
