#include "cucumber_cpp/library/runtime/Worker.hpp"
#include "cucumber/gherkin/app.hpp"
#include "cucumber/gherkin/exceptions.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/gherkin/pickle_compiler.hpp"
#include "cucumber/gherkin/utils.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/parameter_type.hpp"
#include "cucumber/messages/parse_error.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber/messages/source.hpp"
#include "cucumber/messages/source_reference.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/step_definition_pattern.hpp"
#include "cucumber/messages/step_match_argument.hpp"
#include "cucumber/messages/suggestion.hpp"
#include "cucumber/messages/test_case.hpp"
#include "cucumber/messages/test_case_finished.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_run_finished.hpp"
#include "cucumber/messages/test_run_hook_finished.hpp"
#include "cucumber/messages/test_run_hook_started.hpp"
#include "cucumber/messages/test_run_started.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber/messages/timestamp.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/assemble/AssembleTestSuites.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestCase.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/runtime/TestCaseRunner.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Timestamp.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/tag_expression/Parser.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <algorithm>
#include <any>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <map>
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
#include <variant>
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

        cucumber::messages::test_step_result GetWorstTestStepResult(std::span<const cucumber::messages::test_step_result> testStepResults)
        {
            if (testStepResults.empty())
                return { .status = cucumber::messages::test_step_result_status::PASSED };
            return *std::ranges::max_element(testStepResults, compare);
        }

        inline std::set<cucumber::messages::test_step_result_status> failingStatuses{
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

        if (GetWorstTestStepResult(results).status != cucumber::messages::test_step_result_status::PASSED)
            throw std::runtime_error("Failed before all hook");

        return results;
    }

    std::vector<cucumber::messages::test_step_result> Worker::RunAfterAllHooks()
    {
        std::vector<cucumber::messages::test_step_result> results;
        const auto ids = supportCodeLibrary.hookRegistry.FindIds(HookType::afterAll);
        for (const auto& id : ids)
            results.emplace_back(std::move(RunTestHook(id, programContext)));

        if (GetWorstTestStepResult(results).status != cucumber::messages::test_step_result_status::PASSED)
            throw std::runtime_error("Failed after all hook");

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

        if (GetWorstTestStepResult(results).status != cucumber::messages::test_step_result_status::PASSED)
            throw std::runtime_error("Failed before feature hook");

        return results;
    }

    std::vector<cucumber::messages::test_step_result> Worker::RunAfterTestSuiteHooks(const cucumber::messages::feature& feature, Context& context)
    {
        std::vector<cucumber::messages::test_step_result> results;
        const auto ids = supportCodeLibrary.hookRegistry.FindIds(HookType::afterFeature, feature.tags);
        for (const auto& id : ids)
            results.emplace_back(std::move(RunTestHook(id, context)));

        if (GetWorstTestStepResult(results).status != cucumber::messages::test_step_result_status::PASSED)
            throw std::runtime_error("Failed after feature hook");

        return results;
    }

    cucumber::messages::test_step_result Worker::RunTestHook(std::string id, Context& context)
    {
        const auto& definition = supportCodeLibrary.hookRegistry.GetDefinitionById(id);
        const auto testRunHookStartedid = idGenerator->next_id();

        broadcaster.BroadcastEvent({ .test_run_hook_started = cucumber::messages::test_run_hook_started{
                                         .id = testRunHookStartedid,
                                         .test_run_started_id = std::string{ testRunStartedId },
                                         .hook_id = definition.hook.id,
                                         .timestamp = support::TimestampNow(),
                                     } });

        auto result = definition.factory(context)->ExecuteAndCatchExceptions();

        broadcaster.BroadcastEvent({ .test_run_hook_finished = cucumber::messages::test_run_hook_finished{
                                         .test_run_hook_started_id = testRunHookStartedid,
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
