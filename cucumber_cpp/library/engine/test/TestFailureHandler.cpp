
#include "cucumber_cpp/library/engine/FailureHandler.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/test_helper/ContextManagerInstance.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cucumber_cpp::library::engine
{
    struct TestFailureHandler : testing::Test
    {
    };

    TEST_F(TestFailureHandler, ConstructThisNeedsToBeExtendedAndSeparatedWorkInProgress)
    {
        test_helper::ContextManagerInstance contextManager;
        report::ReportForwarderImpl reportHandler{ contextManager };
        TestAssertionHandlerImpl testAssertionHandler{ contextManager, reportHandler };

        ASSERT_THAT(contextManager.StepContext().ExecutionStatus(), testing::Eq(Result::passed));

        CucumberAssertHelper assertHelper{ testing::TestPartResult::kNonFatalFailure, __FILE__, __LINE__, "message" };
        ASSERT_THAT(contextManager.StepContext().ExecutionStatus(), testing::Eq(Result::passed));

        assertHelper = testing::Message("testing::Message") << "user message";
        ASSERT_THAT(contextManager.StepContext().ExecutionStatus(), testing::Eq(Result::failed));
    }
}
