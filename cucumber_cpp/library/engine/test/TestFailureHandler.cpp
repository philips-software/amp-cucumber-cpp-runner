
#include "cucumber_cpp/library/engine/FailureHandler.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/test_helper/ContextManagerInstance.hpp"
#include "cucumber_cpp/library/report/test_helper/ReportForwarderMock.hpp"
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cucumber_cpp::library::engine
{
    namespace
    {

        struct TestFailureHandler : testing::Test
        {

            void Error(const char* failure)
            {
                googleTestEventListener.OnTestPartResult({ testing::TestPartResult::Type::kFatalFailure, "custom_file.cpp", 0, failure });
            }

            test_helper::ContextManagerInstance contextManager;
            report::test_helper::ReportForwarderMock reportHandler{ contextManager };
            TestAssertionHandlerImpl testAssertionHandler{ contextManager, reportHandler };

        private:
            GoogleTestEventListener googleTestEventListener{ testAssertionHandler };
        };
    }

    TEST_F(TestFailureHandler, SetContextToFailed)
    {
        ASSERT_THAT(contextManager.CurrentContext().ExecutionStatus(), testing::Eq(Result::passed));
        ASSERT_THAT(contextManager.ProgramContext().ExecutionStatus(), testing::Eq(Result::passed));
        ASSERT_THAT(contextManager.FeatureContext().ExecutionStatus(), testing::Eq(Result::passed));
        ASSERT_THAT(contextManager.RuleContext().ExecutionStatus(), testing::Eq(Result::passed));
        ASSERT_THAT(contextManager.ScenarioContext().ExecutionStatus(), testing::Eq(Result::passed));
        ASSERT_THAT(contextManager.StepContext().ExecutionStatus(), testing::Eq(Result::passed));

        Error("failure");

        EXPECT_THAT(contextManager.CurrentContext().ExecutionStatus(), testing::Eq(Result::failed));
        EXPECT_THAT(contextManager.ProgramContext().ExecutionStatus(), testing::Eq(Result::failed));
        EXPECT_THAT(contextManager.FeatureContext().ExecutionStatus(), testing::Eq(Result::failed));
        EXPECT_THAT(contextManager.RuleContext().ExecutionStatus(), testing::Eq(Result::failed));
        EXPECT_THAT(contextManager.ScenarioContext().ExecutionStatus(), testing::Eq(Result::failed));
        EXPECT_THAT(contextManager.StepContext().ExecutionStatus(), testing::Eq(Result::failed));
    }

    TEST_F(TestFailureHandler, ReportFailureMessage)
    {
        EXPECT_CALL(reportHandler, Failure("Failure Message", testing::_, testing::_, testing::_));

        Error("Failure Message");
    }
}
