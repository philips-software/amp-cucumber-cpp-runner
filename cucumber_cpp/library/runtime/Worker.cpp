#include "cucumber_cpp/library/runtime/Worker.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/test_run_hook_finished.hpp"
#include "cucumber/messages/test_run_hook_started.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestCase.hpp"
#include "cucumber_cpp/library/runtime/TestCaseRunner.hpp"
#include "cucumber_cpp/library/support/HookRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include "cucumber_cpp/library/util/TransformHookData.hpp"
#include "cucumber_cpp/library/util/TransformPickleTag.hpp"
#include "cucumber_cpp/library/util/TransformTag.hpp"
#include "cucumber_cpp/library/util/TransformTestRunHookStarted.hpp"
#include "cucumber_cpp/library/util/TransformTestStepResult.hpp"
#include "fmt/format.h"
#include <cstddef>
#include <memory>
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
        const inline std::set<cucumber::messages::test_step_result_status> failingStatuses{
            cucumber::messages::test_step_result_status::AMBIGUOUS,
            cucumber::messages::test_step_result_status::FAILED,
            cucumber::messages::test_step_result_status::UNDEFINED,
        };

        std::size_t RetriesForPickle(const cucumber::messages::pickle& pickle, const support::RunOptions::Runtime& options)
        {
            if (options.retry != 0 && options.retryTagExpression->Evaluate(util::TransformPickleTags(pickle.tags)))
                return options.retry;
            else
                return 0;
        }
    }

    Worker::Worker(std::string_view testRunStartedId,
        util::Broadcaster& broadcaster,
        cucumber::gherkin::id_generator_ptr idGenerator,
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

    std::vector<cucumber::messages::test_step_result> Worker::RunBeforeAllHooks()
    {
        std::vector<cucumber::messages::test_step_result> results;
        const auto ids = supportCodeLibrary.hookRegistry.FindIds(util::HookType::beforeAll);

        results.reserve(ids.size());

        for (const auto& id : ids)
            results.emplace_back(RunTestHook(id, programContext));

        return results;
    }

    std::vector<cucumber::messages::test_step_result> Worker::RunAfterAllHooks()
    {
        std::vector<cucumber::messages::test_step_result> results;
        auto ids = supportCodeLibrary.hookRegistry.FindIds(util::HookType::afterAll);
        for (const auto& id : ids | std::views::reverse)
            results.emplace_back(RunTestHook(id, programContext));

        return results;
    }

    bool Worker::RunTestCase(const cucumber::messages::gherkin_document& gherkinDocument, const assemble::AssembledTestCase& assembledTestCase, Context& testSuiteContext, bool failing)
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

    std::vector<cucumber::messages::test_step_result> Worker::RunBeforeTestSuiteHooks(const cucumber::messages::feature& feature, Context& context)
    {
        std::vector<cucumber::messages::test_step_result> results;
        const auto ids = supportCodeLibrary.hookRegistry.FindIds(util::HookType::beforeFeature, util::TransformTags(feature.tags));
        results.reserve(ids.size());

        for (const auto& id : ids)
            results.emplace_back(RunTestHook(id, context));

        return results;
    }

    std::vector<cucumber::messages::test_step_result> Worker::RunAfterTestSuiteHooks(const cucumber::messages::feature& feature, Context& context)
    {
        std::vector<cucumber::messages::test_step_result> results;
        const auto ids = supportCodeLibrary.hookRegistry.FindIds(util::HookType::afterFeature, util::TransformTags(feature.tags));
        results.reserve(ids.size());

        for (const auto& id : ids)
            results.emplace_back(RunTestHook(id, context));

        return results;
    }

    cucumber::messages::test_step_result Worker::RunTestHook(const std::string& id, Context& context)
    {
        const auto& definition = supportCodeLibrary.hookRegistry.GetDefinitionById(id);
        const auto testRunHookStartedId = idGenerator->next_id();

        const auto testRunHookStarted = cucumber::messages::test_run_hook_started{
            .id = testRunHookStartedId,
            .test_run_started_id = std::string{ testRunStartedId },
            .hook_id = definition.data.id,
            .timestamp = util::TimestampNow(),
        };

        broadcaster.BroadcastEvent(cucumber::messages::envelope{ .test_run_hook_started = testRunHookStarted });

        cucumber::messages::test_step_result result{ .duration{ .seconds = 0, .nanos = 0 }, .status = cucumber::messages::test_step_result_status::SKIPPED };
        if (!options.dryRun)
        {
            result = util::TransformTestStepResult(definition.factory(broadcaster, context, util::TransformTestRunHookStarted(testRunHookStarted), false)->ExecuteAndCatchExceptions());

            if (result.status != cucumber::messages::test_step_result_status::PASSED && options.failGlobalHookFast)
                throw GlobalHookError{ fmt::format("Global Hook Failed: {}\nresult:{}", util::TransformHookData(definition.data).to_string(), result.to_string()) };
        }

        broadcaster.BroadcastEvent(cucumber::messages::envelope{ .test_run_hook_finished = cucumber::messages::test_run_hook_finished{
                                                                     .test_run_hook_started_id = testRunHookStartedId,
                                                                     .result = result,
                                                                     .timestamp = util::TimestampNow(),
                                                                 } });

        return result;
    }

    bool Worker::IsStatusFailed(cucumber::messages::test_step_result_status status) const
    {
        if (options.dryRun)
            return false;

        if (options.strict && status == cucumber::messages::test_step_result_status::PENDING)
            return true;

        return failingStatuses.contains(status);
    }
}
