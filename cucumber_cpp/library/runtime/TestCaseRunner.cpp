
#include "cucumber_cpp/library/runtime/TestCaseRunner.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/messages/Duration.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/GherkinDocument.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber/messages/PickleStep.hpp"
#include "cucumber/messages/StepMatchArgumentsList.hpp"
#include "cucumber/messages/Suggestion.hpp"
#include "cucumber/messages/TestCase.hpp"
#include "cucumber/messages/TestCaseFinished.hpp"
#include "cucumber/messages/TestCaseStarted.hpp"
#include "cucumber/messages/TestStep.hpp"
#include "cucumber/messages/TestStepFinished.hpp"
#include "cucumber/messages/TestStepResult.hpp"
#include "cucumber/messages/TestStepResultStatus.hpp"
#include "cucumber/messages/TestStepStarted.hpp"
#include "cucumber/messages/Timestamp.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/runtime/NestedTestCaseRunner.hpp"
#include "cucumber_cpp/library/support/HookRegistry.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber_cpp/library/util/GetWorstTestStepResult.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include "cucumber_cpp/library/util/ScenarioInfo.hpp"
#include "cucumber_cpp/library/util/TestStepResult.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include "cucumber_cpp/library/util/TransformDocString.hpp"
#include "cucumber_cpp/library/util/TransformPickleTag.hpp"
#include "cucumber_cpp/library/util/TransformStepMatchArgumentsList.hpp"
#include "cucumber_cpp/library/util/TransformTable.hpp"
#include "cucumber_cpp/library/util/TransformTestStepResult.hpp"
#include "cucumber_cpp/library/util/TransformTestStepStarted.hpp"
#include <algorithm>
#include <cstddef>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::runtime
{
    namespace
    {
        cucumber::messages::TestStepResult InvokeStep(const util::BodyFactory& bodyFactory, const cucumber::messages::StepMatchArgumentsList& args)
        {
            return util::TransformTestStepResult(util::ConstructAndExecute(bodyFactory, util::StepMatchArgumentsListToExecuteArgs(args)));
        }

        std::optional<util::ScenarioInfo> MakeScenarioInfo(const cucumber::messages::Pickle& pickle)
        {
            return std::make_optional<util::ScenarioInfo>(pickle.name, util::TransformPickleTags(pickle.tags));
        }
    }

    TestCaseRunner::TestCaseRunner(util::Broadcaster& broadcaster,
        cucumber::gherkin::IdGeneratorPtr idGenerator,
        const cucumber::messages::GherkinDocument& gherkinDocument,
        const cucumber::messages::Pickle& pickle,
        const cucumber::messages::TestCase& testCase,
        std::size_t retries,
        bool skip,
        support::SupportCodeLibrary& supportCodeLibrary,
        Context& testSuiteContext)
        : broadcaster{ broadcaster }
        , idGenerator{ std::move(idGenerator) }
        , gherkinDocument{ gherkinDocument }
        , pickle{ pickle }
        , testCase{ testCase }
        , maximumAttempts{ 1 + (skip ? 0 : retries) }
        , skip{ skip }
        , supportCodeLibrary{ supportCodeLibrary }
        , testSuiteContext{ testSuiteContext }
    {}

    cucumber::messages::TestStepResultStatus TestCaseRunner::Run()
    {
        for (std::size_t attempt = 0; attempt < maximumAttempts; ++attempt)
        {
            testStepResults.clear();

            if (RunAttempt(attempt, (attempt + 1) < maximumAttempts))
                continue;

            return GetWorstStepResult().status;
        }

        return cucumber::messages::TestStepResultStatus::UNKNOWN;
    }

    bool TestCaseRunner::RunAttempt(std::size_t attempt, bool moreAttemptsAvailable)
    {
        Context testCaseContext{ &testSuiteContext };
        const auto currentTestCaseStartedId = idGenerator->NextId();
        bool willRetry = false;

        broadcaster.BroadcastEvent([this, attempt, &currentTestCaseStartedId](cucumber::messages::Envelope& envelope)
            {
                cucumber::messages::TestCaseStarted testCaseStarted;
                testCaseStarted.attempt = attempt;
                testCaseStarted.id = currentTestCaseStartedId;
                testCaseStarted.testCaseId = testCase.id;
                testCaseStarted.timestamp = util::TimestampNow();
                envelope.testCaseStarted = std::move(testCaseStarted);
            });

        bool seenSteps = false;
        bool error = false;

        for (const auto& testStep : testCase.testSteps)
        {
            cucumber::messages::TestStepStarted testStepStarted;
            testStepStarted.testCaseStartedId = currentTestCaseStartedId;
            testStepStarted.testStepId = testStep.id;
            testStepStarted.timestamp = util::TimestampNow();
            broadcaster.BroadcastEvent([&testStepStarted](cucumber::messages::Envelope& envelope)
                {
                    envelope.testStepStarted = testStepStarted;
                });

            cucumber::messages::TestStepResult testStepResult;

            if (testStep.hookId)
            {
                testStepResult = RunHook(supportCodeLibrary.hookRegistry.GetDefinitionById(testStep.hookId.value()), !seenSteps, testCaseContext, testStepStarted, error);
            }
            else
            {
                auto pickleStepIter = std::ranges::find_if(pickle.steps, [&](const auto& pickleStep)
                    {
                        return pickleStep.id == testStep.pickleStepId.value();
                    });
                testStepResult = RunStep(*pickleStepIter, testStep, testCaseContext, testStepStarted);
                seenSteps = true;

                if (testStepResult.message.has_value() || testStepResult.exception.has_value())
                    error = true;
            }
            testStepResults.emplace_back(testStepResult);

            broadcaster.BroadcastEvent([&currentTestCaseStartedId, &testStep, &testStepResult](cucumber::messages::Envelope& envelope)
                {
                    cucumber::messages::TestStepFinished testStepFinished;
                    testStepFinished.testCaseStartedId = currentTestCaseStartedId;
                    testStepFinished.testStepId = testStep.id;
                    testStepFinished.testStepResult = testStepResult;
                    testStepFinished.timestamp = util::TimestampNow();
                    envelope.testStepFinished = std::move(testStepFinished);
                });
        }

        willRetry = GetWorstStepResult().status == cucumber::messages::TestStepResultStatus::FAILED && moreAttemptsAvailable;

        broadcaster.BroadcastEvent([&currentTestCaseStartedId, &willRetry](cucumber::messages::Envelope& envelope)
            {
                cucumber::messages::TestCaseFinished testCaseFinished;
                testCaseFinished.testCaseStartedId = currentTestCaseStartedId;
                testCaseFinished.timestamp = util::TimestampNow();
                testCaseFinished.willBeRetried = willRetry;
                envelope.testCaseFinished = std::move(testCaseFinished);
            });

        return willRetry;
    }

    cucumber::messages::TestStepResult TestCaseRunner::RunHook(const support::HookRegistry::Definition& hookDefinition, bool isBeforeHook, Context& testCaseContext, const cucumber::messages::TestStepStarted& testStepStarted, bool hasError)
    {
        if (ShouldSkipHook(isBeforeHook))
        {
            cucumber::messages::TestStepResult testStepResult{};
            testStepResult.status = cucumber::messages::TestStepResultStatus::SKIPPED;
            return testStepResult;
        }

        const util::BodyFactory bodyFactory = [&hookDefinition, this, &testCaseContext, &testStepStarted, hasError](util::TestStepResult& testStepResult)
        {
            return hookDefinition.factory(testStepResult, broadcaster, testCaseContext, util::TransformTestStepStarted(testStepStarted), MakeScenarioInfo(pickle), hasError);
        };

        return InvokeStep(bodyFactory, {});
    }

    std::vector<cucumber::messages::TestStepResult> TestCaseRunner::RunStepHooks(const cucumber::messages::PickleStep& /*pickleStep*/, util::HookType hookType, Context& testCaseContext, const cucumber::messages::TestStepStarted& testStepStarted)
    {
        auto ids = supportCodeLibrary.hookRegistry.FindIds(hookType, util::TransformPickleTags(pickle.tags));
        std::vector<cucumber::messages::TestStepResult> results;
        results.reserve(ids.size());

        for (const auto& id : ids)
        {
            const auto& definition = supportCodeLibrary.hookRegistry.GetDefinitionById(id);
            const auto bodyFactory = [&definition, this, &testCaseContext, &testStepStarted](util::TestStepResult& testStepResult)
            {
                return definition.factory(testStepResult, broadcaster, testCaseContext, util::TransformTestStepStarted(testStepStarted), MakeScenarioInfo(pickle), false);
            };

            results.emplace_back(InvokeStep(bodyFactory, {}));
        }

        return results;
    }

    cucumber::messages::TestStepResult TestCaseRunner::RunStep(const cucumber::messages::PickleStep& pickleStep, const cucumber::messages::TestStep& testStep, Context& testCaseContext, const cucumber::messages::TestStepStarted& testStepStarted)
    {
        auto stepDefinitions = (*testStep.stepDefinitionIds) | std::views::transform([this](const std::string& id) -> const support::StepRegistry::Definition&
                                                                   {
                                                                       return supportCodeLibrary.stepRegistry.GetDefinitionById(id);
                                                                   });

        if (const auto count = testStep.stepDefinitionIds->size(); count == 0)
        {
            broadcaster.BroadcastEvent([this, &pickleStep](cucumber::messages::Envelope& envelope)
                {
                    cucumber::messages::Suggestion suggestion;
                    suggestion.id = idGenerator->NextId();
                    suggestion.pickleStepId = pickleStep.id;
                    envelope.suggestion = std::move(suggestion);
                });

            cucumber::messages::TestStepResult testStepResult{};
            testStepResult.status = cucumber::messages::TestStepResultStatus::UNDEFINED;
            return testStepResult;
        }
        else if (count > 1)
        {
            cucumber::messages::TestStepResult testStepResult{};
            testStepResult.message = "Ambiguous step definitions";
            testStepResult.status = cucumber::messages::TestStepResultStatus::AMBIGUOUS;
            return testStepResult;
        }
        else if (IsSkippingSteps())
        {
            cucumber::messages::TestStepResult testStepResult{};
            testStepResult.status = cucumber::messages::TestStepResultStatus::SKIPPED;
            return testStepResult;
        }

        auto stepResults = RunStepHooks(pickleStep, util::HookType::beforeStep, testCaseContext, testStepStarted);

        if (util::GetWorstTestStepResult(stepResults).status != cucumber::messages::TestStepResultStatus::FAILED)
        {
            const auto dataTable = (pickleStep.argument && pickleStep.argument->dataTable) ? std::make_optional(*pickleStep.argument->dataTable) : std::nullopt;
            const auto docString = (pickleStep.argument && pickleStep.argument->docString) ? std::make_optional(*pickleStep.argument->docString) : std::nullopt;

            const auto& definition = stepDefinitions.front();

            NestedTestCaseRunner nestedTestCaseRunner{ 0, supportCodeLibrary, broadcaster, testCaseContext, util::TransformTestStepStarted(testStepStarted) };

            const util::BodyFactory bodyFactory = [this, &definition, &nestedTestCaseRunner, &testCaseContext, &testStepStarted, &dataTable, &docString](util::TestStepResult& testStepResult)
            {
                return definition.factory(testStepResult, nestedTestCaseRunner, broadcaster, testCaseContext, util::TransformTestStepStarted(testStepStarted), util::TransformTable(dataTable), util::TransformDocString(docString));
            };

            stepResults.push_back(InvokeStep(bodyFactory, testStep.stepMatchArgumentsLists->front()));
        }

        const auto afterStepHookResults = RunStepHooks(pickleStep, util::HookType::afterStep, testCaseContext, testStepStarted);
        stepResults.reserve(stepResults.size() + afterStepHookResults.size());
        stepResults.insert(stepResults.end(), afterStepHookResults.begin(), afterStepHookResults.end());

        auto finalStepResult = util::GetWorstTestStepResult(stepResults);

        cucumber::messages::Duration finalDuration{};
        for (const auto& stepResult : stepResults)
            finalDuration += stepResult.duration;

        finalStepResult.duration = finalDuration;
        return finalStepResult;
    }

    cucumber::messages::TestStepResult TestCaseRunner::GetWorstStepResult() const
    {
        if (testStepResults.empty())
        {
            cucumber::messages::TestStepResult testStepResult{};
            testStepResult.status = skip ? cucumber::messages::TestStepResultStatus::SKIPPED : cucumber::messages::TestStepResultStatus::PASSED;
            return testStepResult;
        }

        return util::GetWorstTestStepResult(testStepResults);
    }

    bool TestCaseRunner::ShouldSkipHook(bool isBeforeHook) const
    {
        return skip || (IsSkippingSteps() && isBeforeHook);
    }

    bool TestCaseRunner::IsSkippingSteps() const
    {
        return GetWorstStepResult().status != cucumber::messages::TestStepResultStatus::PASSED;
    }
}
