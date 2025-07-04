#include "cucumber_cpp/library/TraceTime.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/test_helper/ContextManagerInstance.hpp"
#include "cucumber_cpp/library/report/StdOutReport.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <chrono>
#include <gtest/gtest.h>

namespace cucumber_cpp::library::report
{
    struct TestStdOutReport : testing::Test
    {
        engine::test_helper::ContextManagerInstance contextManagerInstance;
        StdOutReport stdOutReport;
    };

    TEST_F(TestStdOutReport, FeatureStartPrintNothing)
    {
        testing::internal::CaptureStdout();
        stdOutReport.FeatureStart(contextManagerInstance.FeatureContext().info);
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::StrEq(""));
    }

    TEST_F(TestStdOutReport, FeatureEndPrintNothing)
    {
        testing::internal::CaptureStdout();
        stdOutReport.FeatureEnd({}, contextManagerInstance.FeatureContext().info, {});
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::StrEq(""));
    }

    TEST_F(TestStdOutReport, RuleStart)
    {
        testing::internal::CaptureStdout();
        stdOutReport.RuleStart(contextManagerInstance.RuleContext().info);
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::StrEq(""));
    }

    TEST_F(TestStdOutReport, RuleEndPrintNothing)
    {
        testing::internal::CaptureStdout();
        stdOutReport.RuleEnd({}, contextManagerInstance.RuleContext().info, {});
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::StrEq(""));
    }

    TEST_F(TestStdOutReport, ScenarioStart)
    {
        testing::internal::CaptureStdout();
        stdOutReport.ScenarioStart(contextManagerInstance.ScenarioContext().info);
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::StrEq("\nScenarioInfo"));
    }

    TEST_F(TestStdOutReport, ScenarioEndPassed)
    {
        testing::internal::CaptureStdout();
        stdOutReport.ScenarioEnd(engine::Result::passed, contextManagerInstance.ScenarioContext().info, {});
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::HasSubstr("\n\\->"));
        EXPECT_THAT(capture, testing::HasSubstr("done (0ns)"));
    }

    TEST_F(TestStdOutReport, ScenarioEndPassedTimeScale)
    {
        auto testDuration = [this](TraceTime::Duration duration, const char* time)
        {
            testing::internal::CaptureStdout();
            stdOutReport.ScenarioEnd(engine::Result::passed, contextManagerInstance.ScenarioContext().info, duration);
            const auto capture{ testing::internal::GetCapturedStdout() };
            EXPECT_THAT(capture, testing::HasSubstr(time));
        };

        testDuration(std::chrono::nanoseconds{ 1 }, "1ns");
        testDuration(std::chrono::microseconds{ 2 }, "2us");
        testDuration(std::chrono::milliseconds{ 3 }, "3ms");
        testDuration(std::chrono::seconds{ 4 }, "4s");
        testDuration(std::chrono::minutes{ 5 }, "5m");
        testDuration(std::chrono::hours{ 6 }, "6h");
    }

    TEST_F(TestStdOutReport, ScenarioEndSkipped)
    {
        testing::internal::CaptureStdout();
        stdOutReport.ScenarioEnd(engine::Result::skipped, contextManagerInstance.ScenarioContext().info, {});
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::HasSubstr("\n\\->"));
        EXPECT_THAT(capture, testing::HasSubstr("skipped (0ns)"));
    }

    TEST_F(TestStdOutReport, ScenarioEndPending)
    {
        testing::internal::CaptureStdout();
        stdOutReport.ScenarioEnd(engine::Result::pending, contextManagerInstance.ScenarioContext().info, {});
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::HasSubstr("\n\\->"));
        EXPECT_THAT(capture, testing::HasSubstr("pending (0ns)"));
    }

    TEST_F(TestStdOutReport, ScenarioEndUndefined)
    {
        testing::internal::CaptureStdout();
        stdOutReport.ScenarioEnd(engine::Result::undefined, contextManagerInstance.ScenarioContext().info, {});
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::HasSubstr("\n\\->"));
        EXPECT_THAT(capture, testing::HasSubstr("undefined (0ns)"));
    }

    TEST_F(TestStdOutReport, ScenarioEndAmbiguous)
    {
        testing::internal::CaptureStdout();
        stdOutReport.ScenarioEnd(engine::Result::ambiguous, contextManagerInstance.ScenarioContext().info, {});
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::HasSubstr("\n\\->"));
        EXPECT_THAT(capture, testing::HasSubstr("ambiguous (0ns)"));
    }

    TEST_F(TestStdOutReport, ScenarioEndFailed)
    {
        testing::internal::CaptureStdout();
        stdOutReport.ScenarioEnd(engine::Result::failed, contextManagerInstance.ScenarioContext().info, {});
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::HasSubstr("\n\\->"));
        EXPECT_THAT(capture, testing::HasSubstr("failed (0ns)"));
    }

    TEST_F(TestStdOutReport, StepMissing)
    {
        testing::internal::CaptureStdout();
        stdOutReport.StepMissing("missing step text");
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::HasSubstr("Step missing: \"missing step text\""));
    }

    TEST_F(TestStdOutReport, StepAmbiguous)
    {
        testing::internal::CaptureStdout();
        // stdOutReport.StepAmbiguous();
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::StrEq(""));
    }

    TEST_F(TestStdOutReport, StepSkipped)
    {
        testing::internal::CaptureStdout();
        stdOutReport.StepSkipped(contextManagerInstance.StepContext().info);
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::HasSubstr("\n| "));
        EXPECT_THAT(capture, testing::HasSubstr("skipped Given StepInfo"));
    }

    TEST_F(TestStdOutReport, StepStart)
    {
        testing::internal::CaptureStdout();
        stdOutReport.StepStart(contextManagerInstance.StepContext().info);
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::StrEq("\n| Given StepInfo"));
    }

    TEST_F(TestStdOutReport, NestedStepStart)
    {
        testing::internal::CaptureStdout();
        stdOutReport.StepStart(contextManagerInstance.StepContext().info);
        testing::internal::GetCapturedStdout();

        testing::internal::CaptureStdout();
        stdOutReport.StepStart(contextManagerInstance.StepContext().info);
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::StrEq("\n| | Given StepInfo"));
    }

    TEST_F(TestStdOutReport, StepEnd)
    {
        testing::internal::CaptureStdout();
        stdOutReport.StepStart(contextManagerInstance.StepContext().info);
        testing::internal::GetCapturedStdout();

        testing::internal::CaptureStdout();
        stdOutReport.StepEnd(engine::Result::passed, contextManagerInstance.StepContext().info, {});
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::HasSubstr("\n| \\-> "));
        EXPECT_THAT(capture, testing::HasSubstr("done (0ns)"));
    }

    TEST_F(TestStdOutReport, StepEndNested)
    {
        testing::internal::CaptureStdout();
        stdOutReport.StepStart(contextManagerInstance.StepContext().info);
        stdOutReport.StepStart(contextManagerInstance.StepContext().info);
        testing::internal::GetCapturedStdout();

        testing::internal::CaptureStdout();
        stdOutReport.StepEnd(engine::Result::passed, contextManagerInstance.StepContext().info, {});
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::HasSubstr("\n| | \\-> "));
        EXPECT_THAT(capture, testing::HasSubstr("done (0ns)"));
    }

    TEST_F(TestStdOutReport, Failure)
    {
        testing::internal::CaptureStdout();
        stdOutReport.Failure("My Failure", "file.cpp", 1, 1);
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::HasSubstr("\nFailure @ ./file.cpp:1:1:\nMy Failure"));
    }

    TEST_F(TestStdOutReport, FailureWithoutColumn)
    {
        testing::internal::CaptureStdout();
        stdOutReport.Failure("My Failure", "file.cpp", 1);
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::HasSubstr("\nFailure @ ./file.cpp:1:\nMy Failure"));
    }

    TEST_F(TestStdOutReport, Error)
    {
        testing::internal::CaptureStdout();
        stdOutReport.Error("My Error", "file.cpp", 1, 1);
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::HasSubstr("\nError @ ./file.cpp:1:1:\nMy Error"));
    }

    TEST_F(TestStdOutReport, ErrorWithoutColumn)
    {
        testing::internal::CaptureStdout();
        stdOutReport.Error("My Error", "file.cpp", 1);
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::HasSubstr("\nError @ ./file.cpp:1:\nMy Error"));
    }

    TEST_F(TestStdOutReport, Trace)
    {
        testing::internal::CaptureStdout();
        stdOutReport.Trace("Traces a message without any formatting");
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::StrEq("Traces a message without any formatting"));
    }

    TEST_F(TestStdOutReport, Summary)
    {
        testing::internal::CaptureStdout();
        stdOutReport.ScenarioStart(contextManagerInstance.ScenarioContext().info);
        stdOutReport.ScenarioStart(contextManagerInstance.ScenarioContext().info);
        stdOutReport.ScenarioStart(contextManagerInstance.ScenarioContext().info);

        stdOutReport.ScenarioEnd(engine::Result::passed, contextManagerInstance.ScenarioContext().info, {});
        stdOutReport.ScenarioEnd(engine::Result::skipped, contextManagerInstance.ScenarioContext().info, {});
        stdOutReport.ScenarioEnd(engine::Result::failed, contextManagerInstance.ScenarioContext().info, {});

        testing::internal::GetCapturedStdout();

        testing::internal::CaptureStdout();
        stdOutReport.Summary({});
        const auto capture{ testing::internal::GetCapturedStdout() };
        EXPECT_THAT(capture, testing::StrEq("\n====================summary===================="
                                            "\nduration: 0ns"
                                            "\ntests   : 1/3 passed"
                                            "\n"
                                            "\nfailed tests:"
                                            "\n\"\":0:0 : \"ScenarioInfo\""
                                            "\n\"\":0:0 : \"ScenarioInfo\""));
    }

}
