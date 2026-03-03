#ifndef UTIL_TRANSFORM_TEST_RUN_HOOK_STARTED_HPP
#define UTIL_TRANSFORM_TEST_RUN_HOOK_STARTED_HPP

#include "cucumber/messages/test_run_hook_started.hpp"
#include "cucumber_cpp/library/util/TestRunHookStarted.hpp"

namespace cucumber_cpp::library::util
{
    TestRunHookStarted TransformTestRunHookStarted(const cucumber::messages::test_run_hook_started& testRunHookStarted);
}

#endif
