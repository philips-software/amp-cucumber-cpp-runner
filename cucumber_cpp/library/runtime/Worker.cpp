#include "cucumber_cpp/library/runtime/Worker.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/test_run_hook_finished.hpp"
#include "cucumber/messages/test_run_hook_started.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestCase.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/runtime/TestCaseRunner.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Timestamp.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/GetWorstTestStepResult.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::runtime
{
    namespace
    {
        cucumber::messages::duration operator+=(cucumber::messages::duration durationA, cucumber::messages::duration durationB)
        {
            const auto seconds = durationA.seconds + durationB.seconds;
            const auto nanos = durationA.nanos + durationB.nanos;

            if (nanos >= support::nanosecondsPerSecond)
                return { seconds + 1, nanos - support::nanosecondsPerSecond };
            else
                return { seconds, nanos };
        }

        const auto to_underlying = [](const auto& value)
        {
            return static_cast<std::underlying_type_t<std::remove_cvref_t<decltype(value)>>>(value);
        };

        const auto compare = [](const cucumber::messages::test_step_result& a, const cucumber::messages::test_step_result& b)
        {
            return to_underlying(a.status) < to_underlying(b.status);
        };

        const inline std::set<cucumber::messages::test_step_result_status> failingStatuses{
            cucumber::messages::test_step_result_status::AMBIGUOUS,
            cucumber::messages::test_step_result_status::FAILED,
            cucumber::messages::test_step_result_status::UNDEFINED,
        };
    }

    Worker::Worker(std::string_view testRunStartedId,
        util::Broadcaster& broadcaster,
        cucumber::gherkin::id_generator_ptr idGenerator,
        const support::RunOptions::Runtime& options,
        support::SupportCodeLibrary supportCodeLibrary,
        Context& programContext)
        : testRunStartedId{ testRunStartedId }
        , broadcaster{ broadcaster }
        , idGenerator{ idGenerator }
        , options{ options }
        , supportCodeLibrary{ supportCodeLibrary }
        , programContext{ programContext }
    {}

    std::vector<cucumber::messages::test_step_result> Worker::RunBeforeAllHooks()
    {
        std::vector<cucumber::messages::test_step_result> results;
        const auto ids = supportCodeLibrary.hookRegistry.FindIds(HookType::beforeAll);
        for (const auto& id : ids)
            results.emplace_back(std::move(RunTestHook(id, programContext)));

        return results;
    }

    std::vector<cucumber::messages::test_step_result> Worker::RunAfterAllHooks()
    {
        std::vector<cucumber::messages::test_step_result> results;
        auto ids = supportCodeLibrary.hookRegistry.FindIds(HookType::afterAll);
        for (const auto& id : ids | std::views::reverse)
            results.emplace_back(std::move(RunTestHook(id, programContext)));

        return results;
    }

    bool Worker::RunTestSuite(const assemble::AssembledTestSuite& assembledTestSuite, bool failing)
    {
        Context testSuiteContext{ &programContext };

        RunBeforeTestSuiteHooks(*assembledTestSuite.gherkinDocument.feature, testSuiteContext);

        auto failed = false;
        for (const auto& assembledTestCase : assembledTestSuite.testCases)
            failed |= !RunTestCase(assembledTestSuite.gherkinDocument, assembledTestCase, testSuiteContext, failing);

        RunAfterTestSuiteHooks(*assembledTestSuite.gherkinDocument.feature, testSuiteContext);

        return !failed;
    }

    bool Worker::RunTestCase(const cucumber::messages::gherkin_document& gherkinDocument, const assemble::AssembledTestCase& assembledTestCase, Context& testSuiteContext, bool failing)
    {
        TestCaseRunner testCaseRunner{
            broadcaster,
            idGenerator,
            gherkinDocument,
            assembledTestCase.pickle,
            assembledTestCase.testCase,
            options.retry,
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
        const auto ids = supportCodeLibrary.hookRegistry.FindIds(HookType::beforeFeature, feature.tags);
        for (const auto& id : ids)
            results.emplace_back(std::move(RunTestHook(id, context)));

        if (util::GetWorstTestStepResult(results).status != cucumber::messages::test_step_result_status::PASSED)
            throw std::runtime_error("Failed before feature hook");

        return results;
    }

    std::vector<cucumber::messages::test_step_result> Worker::RunAfterTestSuiteHooks(const cucumber::messages::feature& feature, Context& context)
    {
        std::vector<cucumber::messages::test_step_result> results;
        const auto ids = supportCodeLibrary.hookRegistry.FindIds(HookType::afterFeature, feature.tags);
        for (const auto& id : ids)
            results.emplace_back(std::move(RunTestHook(id, context)));

        if (util::GetWorstTestStepResult(results).status != cucumber::messages::test_step_result_status::PASSED)
            throw std::runtime_error("Failed after feature hook");

        return results;
    }

    cucumber::messages::test_step_result Worker::RunTestHook(std::string id, Context& context)
    {
        const auto& definition = supportCodeLibrary.hookRegistry.GetDefinitionById(id);
        const auto testRunHookStartedId = idGenerator->next_id();

        const auto testRunHookStarted = cucumber::messages::test_run_hook_started{
            .id = testRunHookStartedId,
            .test_run_started_id = std::string{ testRunStartedId },
            .hook_id = definition.hook.id,
            .timestamp = support::TimestampNow(),
        };

        broadcaster.BroadcastEvent({ .test_run_hook_started = testRunHookStarted });

        auto result = definition.factory(broadcaster, context, testRunHookStarted)->ExecuteAndCatchExceptions();

        broadcaster.BroadcastEvent({ .test_run_hook_finished = cucumber::messages::test_run_hook_finished{
                                         .test_run_hook_started_id = testRunHookStartedId,
                                         .result = result,
                                         .timestamp = support::TimestampNow(),
                                     } });

        return result;
    }

    bool Worker::IsStatusFailed(cucumber::messages::test_step_result_status status)
    {
        if (options.dryRun)
            return false;

        if (options.strict && status == cucumber::messages::test_step_result_status::PENDING)
            return true;

        return failingStatuses.contains(status);
    }

}
