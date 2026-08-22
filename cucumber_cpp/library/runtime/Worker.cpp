#include "cucumber_cpp/library/runtime/Worker.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/messages/Duration.hpp"
#include "cucumber/messages/Feature.hpp"
#include "cucumber/messages/GherkinDocument.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber/messages/TestRunHookFinished.hpp"
#include "cucumber/messages/TestRunHookStarted.hpp"
#include "cucumber/messages/TestStepResult.hpp"
#include "cucumber/messages/TestStepResultStatus.hpp"
#include "cucumber/messages/Timestamp.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestCase.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/runtime/TestCaseRunner.hpp"
#include "cucumber_cpp/library/support/HookRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/GetWorstTestStepResult.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include "cucumber_cpp/library/util/MakeShared.hpp"
#include "cucumber_cpp/library/util/TestStepResult.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include "cucumber_cpp/library/util/TransformHookData.hpp"
#include "cucumber_cpp/library/util/TransformPickleTag.hpp"
#include "cucumber_cpp/library/util/TransformTag.hpp"
#include "cucumber_cpp/library/util/TransformTestRunHookStarted.hpp"
#include "cucumber_cpp/library/util/TransformTestStepResult.hpp"
#include "fmt/format.h"
#include <cstddef>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::runtime
{
    namespace
    {
        const inline std::set failingStatuses{
            cucumber::messages::TestStepResultStatus::AMBIGUOUS,
            cucumber::messages::TestStepResultStatus::FAILED,
            cucumber::messages::TestStepResultStatus::UNDEFINED,
        };

        std::size_t RetriesForPickle(const cucumber::messages::Pickle& pickle, const support::RunOptions::Runtime& options)
        {
            if (options.retry == 0)
                return 0;
            else if (options.retryTagExpression->Evaluate(util::TransformPickleTags(pickle.tags)))
                return options.retry;
            else
                return 0;
        }

        bool IsFailing(cucumber::messages::TestStepResultStatus status, bool dryRun)
        {
            if (dryRun)
                return false;

            return status != cucumber::messages::TestStepResultStatus::PASSED;
        }
    }

    Worker::Worker(std::string_view testRunStartedId,
        util::Broadcaster& broadcaster,
        cucumber::gherkin::IdGeneratorPtr idGenerator,
        const support::RunOptions::Runtime& options,
        support::SupportCodeLibrary& supportCodeLibrary,
        Context& programContext)
        : testRunStartedId{ testRunStartedId }
        , broadcaster{ broadcaster }
        , idGenerator{ std::move(idGenerator) }
        , options{ options }
        , supportCodeLibrary{ supportCodeLibrary }
        , programContext{ programContext }
    {}

    std::vector<cucumber::messages::TestStepResult> Worker::RunBeforeAllHooks()
    {
        std::vector<cucumber::messages::TestStepResult> results;
        const auto ids = supportCodeLibrary.hookRegistry.FindIds(util::HookType::beforeAll);

        results.reserve(ids.size());

        for (const auto& id : ids)
            results.emplace_back(RunTestHook(id, programContext));

        return results;
    }

    std::vector<cucumber::messages::TestStepResult> Worker::RunAfterAllHooks()
    {
        std::vector<cucumber::messages::TestStepResult> results;
        auto ids = supportCodeLibrary.hookRegistry.FindIds(util::HookType::afterAll);
        for (const auto& id : ids | std::views::reverse)
            results.emplace_back(RunTestHook(id, programContext));

        return results;
    }

    bool Worker::RunTestSuite(const assemble::AssembledTestSuite& assembledTestSuite, bool failing)
    {
        Context testSuiteContext{ &programContext };

        auto failed = false;

        if (options.featureHooks)
        {
            const auto beforeHookResults = RunBeforeTestSuiteHooks(**assembledTestSuite.gherkinDocument.feature, testSuiteContext);

            if (IsFailing(util::GetWorstTestStepResult(beforeHookResults).status, options.dryRun))
                failing = true;
        }

        for (const auto& assembledTestCase : assembledTestSuite.testCases)
            failed |= !RunTestCase(assembledTestSuite.gherkinDocument, assembledTestCase, testSuiteContext, failed || failing);

        if (options.featureHooks)
        {
            const auto afterHookResults = RunAfterTestSuiteHooks(**assembledTestSuite.gherkinDocument.feature, testSuiteContext);

            if (IsFailing(util::GetWorstTestStepResult(afterHookResults).status, options.dryRun))
                failing = true;
        }

        return !failed;
    }

    bool Worker::RunTestCase(const cucumber::messages::GherkinDocument& gherkinDocument, const assemble::AssembledTestCase& assembledTestCase, Context& testSuiteContext, bool failing)
    {
        TestCaseRunner testCaseRunner{
            broadcaster,
            idGenerator,
            gherkinDocument,
            assembledTestCase.pickle,
            assembledTestCase.testCase,
            RetriesForPickle(assembledTestCase.pickle, options),
            options.dryRun || (options.failFast && failing),
            supportCodeLibrary,
            testSuiteContext,
        };

        const auto status = testCaseRunner.Run();

        return !IsStatusFailed(status);
    }

    std::vector<cucumber::messages::TestStepResult> Worker::RunBeforeTestSuiteHooks(const cucumber::messages::Feature& feature, Context& context)
    {
        std::vector<cucumber::messages::TestStepResult> results;
        const auto ids = supportCodeLibrary.hookRegistry.FindIds(util::HookType::beforeFeature, util::TransformTags(feature.tags));
        results.reserve(ids.size());

        for (const auto& id : ids)
            results.emplace_back(RunTestHook(id, context));

        return results;
    }

    std::vector<cucumber::messages::TestStepResult> Worker::RunAfterTestSuiteHooks(const cucumber::messages::Feature& feature, Context& context)
    {
        std::vector<cucumber::messages::TestStepResult> results;
        const auto ids = supportCodeLibrary.hookRegistry.FindIds(util::HookType::afterFeature, util::TransformTags(feature.tags));
        results.reserve(ids.size());

        for (const auto& id : ids)
            results.emplace_back(RunTestHook(id, context));

        return results;
    }

    cucumber::messages::TestStepResult Worker::RunTestHook(const std::string& id, Context& context)
    {
        const auto& definition = supportCodeLibrary.hookRegistry.GetDefinitionById(id);
        const auto testRunHookStartedId = idGenerator->NextId();

        const auto testRunHookStarted = cucumber::messages::TestRunHookStarted{
            .id = testRunHookStartedId,
            .testRunStartedId = std::string{ testRunStartedId },
            .hookId = definition.data.id,
            .timestamp = std::make_shared<cucumber::messages::Timestamp>(util::TimestampNow()),
        };

        broadcaster.BroadcastEvent(util::MakeShared(testRunHookStarted));

        cucumber::messages::TestStepResult result{ .duration = std::make_shared<cucumber::messages::Duration>(cucumber::messages::Duration{ .seconds = 0, .nanos = 0 }), .status = cucumber::messages::TestStepResultStatus::SKIPPED };

        if (!options.dryRun)
        {
            const util::BodyFactory bodyFactory = [&definition, this, &context, &testRunHookStarted](util::TestStepResult& testStepResult)
            {
                return definition.factory(testStepResult, broadcaster, context, util::TransformTestRunHookStarted(testRunHookStarted), std::nullopt, false);
            };

            result = util::TransformTestStepResult(util::ConstructAndExecute(bodyFactory));

            if (result.status != cucumber::messages::TestStepResultStatus::PASSED && options.failGlobalHookFast)
                throw GlobalHookError{ fmt::format("Global Hook Failed: {}\nresult:{}", util::TransformHookData(definition.data).to_string(), result.to_string()) };
        }

        broadcaster.BroadcastEvent(util::MakeShared(cucumber::messages::TestRunHookFinished{
            .testRunHookStartedId = testRunHookStartedId,
            .result = util::MakeShared(result),
            .timestamp = util::MakeShared(util::TimestampNow()),
        }));

        return result;
    }

    bool Worker::IsStatusFailed(cucumber::messages::TestStepResultStatus status) const
    {
        if (options.dryRun)
            return false;

        if (options.strict && status == cucumber::messages::TestStepResultStatus::PENDING)
            return true;

        return failingStatuses.contains(status);
    }
}
