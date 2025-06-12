
#include "cucumber_cpp/library/engine/FailureHandler.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/test_helper/ContextManagerInstance.hpp"
#include "cucumber_cpp/library/report/test_helper/ReportForwarderMock.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cucumber_cpp::library::engine
{
    namespace
    {
        void ErrorWithFailureMessage(const char* failure)
        {
            CucumberAssertHelper{ testing::TestPartResult::kNonFatalFailure, "custom_file.cpp", 0, failure } = testing::Message();
        }

        void ErrorWithUserMessage(const char* user)
        {
            CucumberAssertHelper{ testing::TestPartResult::kNonFatalFailure, "custom_file.cpp", 0, "" } = testing::Message() << user;
        }

        void ErrorWithFailureAndUserMessage(const char* failure, const char* user)
        {
            CucumberAssertHelper{ testing::TestPartResult::kNonFatalFailure, "custom_file.cpp", 0, failure } = testing::Message() << user;
        }

        struct TestFailureHandler : testing::Test
        {

            test_helper::ContextManagerInstance contextManager;
            report::test_helper::ReportForwarderMock reportHandler{ contextManager };
            TestAssertionHandlerImpl testAssertionHandler{ contextManager, reportHandler };
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

        ErrorWithFailureMessage("failure");

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

        ErrorWithFailureMessage("Failure Message");
    }

    TEST_F(TestFailureHandler, ReportUserMessage)
    {
        EXPECT_CALL(reportHandler, Failure("User Message", testing::_, testing::_, testing::_));

        ErrorWithUserMessage("User Message");
    }

    TEST_F(TestFailureHandler, ReportFailureAndUserMessage)
    {
        EXPECT_CALL(reportHandler, Failure("Failure Message\nUser Message", testing::_, testing::_, testing::_));

        ErrorWithFailureAndUserMessage("Failure Message", "User Message");
    }
}
