#include "cucumber_cpp/library/runtime/TestCaseRunner.hpp"
#include "cucumber_cpp/Steps.hpp"
#include "cucumber_cpp/library/support/HookRegistry.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/UndefinedParameters.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include "gtest/gtest.h"
#include <cstddef>
#include <cucumber/cucumber-expressions/ParameterRegistry.hpp>
#include <cucumber/gherkin/IdGenerator.hpp>
#include <cucumber/messages/Envelope.hpp>
#include <cucumber/messages/GherkinDocument.hpp>
#include <cucumber/messages/Pickle.hpp>
#include <cucumber/messages/TestCase.hpp>
#include <cucumber/messages/TestCaseFinished.hpp>
#include <cucumber/messages/TestCaseStarted.hpp>
#include <cucumber/messages/TestStepResultStatus.hpp>
#include <memory>
#include <optional>
#include <set>
#include <source_location>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace cucumber_cpp::library::runtime
{
    namespace
    {
        using Status = cucumber::messages::TestStepResultStatus;

        struct AttemptOutcomes
        {
            std::vector<Status> statuses;
            std::size_t executed{ 0 };
        };

        HOOK_BEFORE_SCENARIO()
        {
            auto& outcomes = context.Get<AttemptOutcomes>();
            EXPECT_FALSE(context.Contains("attempt-local"));
            context.InsertAt("attempt-local", true);

            const auto status = outcomes.statuses.at(outcomes.executed++);
            if (status == Status::FAILED)
                throw std::runtime_error{ "controlled failure" };
            if (status == Status::PENDING)
                throw util::StepPending{ "controlled pending", std::source_location::current() };
            if (status == Status::SKIPPED)
                throw util::StepSkipped{ "controlled skip", std::source_location::current() };
        }

        struct TestCaseRunnerTest : testing::Test
        {
        protected:
            void SetUp() override
            {
                suiteContext.InsertRef(outcomes);
                hookRegistry.LoadHooks();
                for (const auto& id : hookRegistry.FindIds(util::HookType::before))
                    testCase.testSteps.push_back({ .hookId = id, .id = idGenerator->NextId() });
            }

        public:
            Status Run(std::size_t retries = 0, std::optional<std::size_t> repeat = std::nullopt, bool skip = false)
            {
                TestCaseRunner runner{ broadcaster, idGenerator, document, pickle, testCase, retries, skip, supportCode, suiteContext, repeat };
                return runner.Run();
            }

            void ExpectAttempts(std::size_t count)
            {
                ASSERT_EQ(started.size(), count);
                ASSERT_EQ(finished.size(), count);
                std::set<std::string> ids;
                for (std::size_t attempt = 0; attempt < count; ++attempt)
                {
                    const auto& start = started.at(attempt);
                    const auto& finish = finished.at(attempt);
                    EXPECT_EQ(std::make_tuple(start.attempt, start.testCaseId, finish.testCaseStartedId, finish.willBeRetried),
                        std::make_tuple(attempt, testCase.id, start.id, attempt + 1 < count));
                    ids.insert(start.id);
                }
                EXPECT_EQ(ids.size(), count);
            }

            cucumber::gherkin::IdGeneratorPtr idGenerator{ cucumber::gherkin::NewIdGenerator() };
            util::TimestampGeneratorSystemClock timestampGenerator;
            util::StopWatchHighResolutionClock stopwatch;
            util::Broadcaster broadcaster;
            cucumber::messages::GherkinDocument document;
            cucumber::messages::Pickle pickle;
            cucumber::messages::TestCase testCase{ .id = "test-case" };
            cucumber::cucumber_expressions::ParameterRegistry parameterRegistry{ {} };
            support::UndefinedParameters undefinedParameters;
            support::HookRegistry hookRegistry{ idGenerator };
            support::StepRegistry stepRegistry{ parameterRegistry, undefinedParameters, idGenerator };
            support::SupportCodeLibrary supportCode{ hookRegistry, stepRegistry, parameterRegistry, undefinedParameters };
            Context suiteContext{ std::make_shared<ContextStorageFactoryImpl>() };
            AttemptOutcomes outcomes;
            std::vector<cucumber::messages::TestCaseStarted> started;
            std::vector<cucumber::messages::TestCaseFinished> finished;
            util::Listener listener{ broadcaster, [this](const cucumber::messages::Envelope& envelope)
                {
                    if (envelope.testCaseStarted)
                        started.push_back(*envelope.testCaseStarted);
                    if (envelope.testCaseFinished)
                        finished.push_back(*envelope.testCaseFinished);
                } };
        };
    }

    TEST_F(TestCaseRunnerTest, RepeatPassesExactlyThreeTimes)
    {
        outcomes.statuses = { Status::PASSED, Status::PASSED, Status::PASSED };
        EXPECT_EQ(Run(0, 3), Status::PASSED);
        EXPECT_EQ(outcomes.executed, 3);
        ExpectAttempts(3);
    }

    TEST_F(TestCaseRunnerTest, RepeatRetainsEarlyFailure)
    {
        outcomes.statuses = { Status::FAILED, Status::PASSED, Status::PASSED };
        EXPECT_EQ(Run(0, 3), Status::FAILED);
        EXPECT_EQ(outcomes.executed, 3);
        ExpectAttempts(3);
    }

    TEST_F(TestCaseRunnerTest, RepeatRetainsMiddleFailure)
    {
        outcomes.statuses = { Status::PASSED, Status::FAILED, Status::PASSED };
        EXPECT_EQ(Run(0, 3), Status::FAILED);
        ExpectAttempts(3);
    }

    TEST_F(TestCaseRunnerTest, RepeatRetainsFinalFailure)
    {
        outcomes.statuses = { Status::PASSED, Status::PASSED, Status::FAILED };
        EXPECT_EQ(Run(0, 3), Status::FAILED);
        ExpectAttempts(3);
    }

    TEST_F(TestCaseRunnerTest, RepeatAllFailuresExactlyThreeTimes)
    {
        outcomes.statuses = { Status::FAILED, Status::FAILED, Status::FAILED };
        EXPECT_EQ(Run(0, 3), Status::FAILED);
        ExpectAttempts(3);
    }

    TEST_F(TestCaseRunnerTest, RepeatPendingAndSkippedOutcomes)
    {
        outcomes.statuses = { Status::PENDING, Status::SKIPPED, Status::PASSED };
        EXPECT_EQ(Run(0, 3), Status::PENDING);
        ExpectAttempts(3);
    }

    TEST_F(TestCaseRunnerTest, RepeatUndefinedSteps)
    {
        testCase.testSteps = { { .id = "step", .pickleStepId = "pickle-step", .stepDefinitionIds = std::vector<std::string>{} } };
        pickle.steps = { { .id = "pickle-step", .text = "undefined" } };
        EXPECT_EQ(Run(0, 3), Status::UNDEFINED);
        ExpectAttempts(3);
    }

    TEST_F(TestCaseRunnerTest, RepeatAmbiguousSteps)
    {
        testCase.testSteps = { { .id = "step", .pickleStepId = "pickle-step", .stepDefinitionIds = std::vector<std::string>{ "first", "second" } } };
        pickle.steps = { { .id = "pickle-step", .text = "ambiguous" } };
        EXPECT_EQ(Run(0, 3), Status::AMBIGUOUS);
        ExpectAttempts(3);
    }

    TEST_F(TestCaseRunnerTest, RetryRecoversOnFirstPass)
    {
        outcomes.statuses = { Status::FAILED, Status::PASSED };
        EXPECT_EQ(Run(5), Status::PASSED);
        ExpectAttempts(2);
    }

    TEST_F(TestCaseRunnerTest, RetryStopsAtLimit)
    {
        outcomes.statuses = { Status::FAILED, Status::FAILED, Status::FAILED };
        EXPECT_EQ(Run(2), Status::FAILED);
        ExpectAttempts(3);
    }

    TEST_F(TestCaseRunnerTest, RetryDoesNotRepeatPending)
    {
        outcomes.statuses = { Status::PENDING };
        EXPECT_EQ(Run(5), Status::PENDING);
        ExpectAttempts(1);
    }

    TEST_F(TestCaseRunnerTest, DefaultAndRepeatOneExecuteOnce)
    {
        outcomes.statuses = { Status::PASSED };
        EXPECT_EQ(Run(), Status::PASSED);
        ExpectAttempts(1);
    }

    TEST_F(TestCaseRunnerTest, ForcedSkipExecutesOnlyOneAttempt)
    {
        EXPECT_EQ(Run(0, 3, true), Status::SKIPPED);
        EXPECT_EQ(outcomes.executed, 0);
        ExpectAttempts(1);
    }

    TEST_F(TestCaseRunnerTest, RejectsInvalidRepeatConfiguration)
    {
        EXPECT_THROW(Run(0, 0), std::invalid_argument);
        EXPECT_THROW(Run(1, 3), std::invalid_argument);
        EXPECT_TRUE(started.empty());
    }
}
