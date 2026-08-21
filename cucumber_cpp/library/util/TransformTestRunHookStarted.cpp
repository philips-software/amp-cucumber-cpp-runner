#include "cucumber_cpp/library/util/TransformTestRunHookStarted.hpp"
#include "cucumber/messages/TestRunHookStarted.hpp"
#include "cucumber_cpp/library/util/TestRunHookStarted.hpp"
#include <string>

namespace cucumber_cpp::library::util
{
    TestRunHookStarted TransformTestRunHookStarted(const cucumber::messages::TestRunHookStarted& testRunHookStarted)
    {
        return {
            .testRunStartedId = testRunHookStarted.id,
        };
    }
}
