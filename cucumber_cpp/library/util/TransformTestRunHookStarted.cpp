#include "cucumber_cpp/library/util/TransformTestRunHookStarted.hpp"
#include "cucumber/messages/test_run_hook_started.hpp"
#include "cucumber_cpp/library/util/TestRunHookStarted.hpp"
#include <string>

namespace cucumber_cpp::library::util
{
    TestRunHookStarted TransformTestRunHookStarted(const cucumber::messages::test_run_hook_started& testRunHookStarted)
    {
        return {
            .testRunStartedId = testRunHookStarted.id,
        };
    }
}
