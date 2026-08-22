#include "cucumber_cpp/library/util/TransformTestStepStarted.hpp"
#include "cucumber/messages/TestStepStarted.hpp"
#include "cucumber_cpp/library/util/TestStepStarted.hpp"

namespace cucumber_cpp::library::util
{
    TestStepStarted TransformTestStepStarted(const cucumber::messages::TestStepStarted& testStepStarted)
    {
        return {
            .testCaseStartedId = testStepStarted.testCaseStartedId,
            .testStepId = testStepStarted.testStepId,
        };
    }
}
