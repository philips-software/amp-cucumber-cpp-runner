#include "cucumber_cpp/library/util/TransformTestStepStarted.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber_cpp/library/util/TestStepStarted.hpp"

namespace cucumber_cpp::library::util
{
    TestStepStarted TransformTestStepStarted(const cucumber::messages::test_step_started& testStepStarted)
    {
        return {
            .testCaseStartedId = testStepStarted.test_case_started_id,
            .testStepId = testStepStarted.test_step_id,
        };
    }
}
