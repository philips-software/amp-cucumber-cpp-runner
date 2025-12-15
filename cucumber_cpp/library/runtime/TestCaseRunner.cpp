
#include "cucumber_cpp/library/runtime/TestCaseRunner.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber/messages/pickle_table_row.hpp"
#include "cucumber/messages/step_match_argument.hpp"
#include "cucumber/messages/suggestion.hpp"
#include "cucumber/messages/test_case.hpp"
#include "cucumber/messages/test_case_finished.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/Duration.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Timestamp.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/GetWorstTestStepResult.hpp"
#include <algorithm>
#include <any>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::runtime
{
    namespace
    {
        // cucumber::messages::duration operator+=(cucumber::messages::duration durationA, cucumber::messages::duration durationB)
        // {
        //     const auto seconds = durationA.seconds + durationB.seconds;
        //     const auto nanos = durationA.nanos + durationB.nanos;

        //     if (nanos >= support::nanosecondsPerSecond)
        //         return { seconds + 1, nanos - support::nanosecondsPerSecond };
        //     else
        //         return { seconds, nanos };
        // }

        std::vector<std::any> BuildExpressionParameters(std::span<const cucumber::messages::step_match_argument> arguments, cucumber_expression::ParameterRegistry& parameterRegistry)
        {
            std::vector<std::any> parameters;

            for (const auto& argument : arguments)
            {
                const auto parameter = parameterRegistry.Lookup(*argument.parameter_type_name);
                parameters.push_back(parameter.converter.toAny(argument.group));
            }

            return parameters;
        }

        std::vector<std::string> BuildRegularParameters(std::span<const cucumber::messages::step_match_argument> arguments)
        {
            const auto transformedArguments = arguments | std::views::transform([](const auto& argument)
                                                              {
                                                                  return argument.group.value.value();
                                                              });
            return { transformedArguments.begin(), transformedArguments.end() };
        }
    }

    TestCaseRunner::TestCaseRunner(util::Broadcaster& broadcaster,
        cucumber::gherkin::id_generator_ptr idGenerator,
        const cucumber::messages::gherkin_document& gherkinDocument,
        const cucumber::messages::pickle& pickle,
        const cucumber::messages::test_case& testCase,
        std::size_t retries,
        bool skip,
        support::SupportCodeLibrary supportCodeLibrary,
        Context& testSuiteContext)
        : broadcaster{ broadcaster }
        , idGenerator{ idGenerator }
        , gherkinDocument{ gherkinDocument }
        , pickle{ pickle }
        , testCase{ testCase }
        , maximumAttempts{ 1 + (skip ? 0 : retries) }
        , skip{ skip }
        , supportCodeLibrary{ supportCodeLibrary }
        , testSuiteContext{ testSuiteContext }
    {}

    cucumber::messages::test_step_result_status TestCaseRunner::Run()
    {
        for (std::size_t attempt = 0; attempt < maximumAttempts; ++attempt)
        {
            testStepResults.clear();

            const auto willRetry = RunAttempt(attempt, (attempt + 1) < maximumAttempts);

            if (willRetry)
                continue;

            return GetWorstStepResult().status;
        }

        return cucumber::messages::test_step_result_status::UNKNOWN;
    }

    bool TestCaseRunner::RunAttempt(std::size_t attempt, bool moreAttemptsAvailable)
    {
        Context testCaseContext{ &testSuiteContext };
        const auto currentTestCaseStartedId = idGenerator->next_id();
        bool willRetry = false;

        broadcaster.BroadcastEvent({ .test_case_started = cucumber::messages::test_case_started{
                                         .attempt = attempt,
                                         .id = currentTestCaseStartedId,
                                         .test_case_id = testCase.id,
                                         .timestamp = support::TimestampNow(),
                                     } });

        bool seenSteps = false;
        bool error = false;

        for (const auto& testStep : testCase.test_steps)
        {
            auto testStepStarted = cucumber::messages::test_step_started{
                .test_case_started_id = currentTestCaseStartedId,
                .test_step_id = testStep.id,
                .timestamp = support::TimestampNow(),
            };
            broadcaster.BroadcastEvent({ .test_step_started = testStepStarted });

            cucumber::messages::test_step_result testStepResult;

            if (testStep.hook_id)
            {
                testStepResult = RunHook(supportCodeLibrary.hookRegistry.GetDefinitionById(testStep.hook_id.value()), !seenSteps, testCaseContext);
            }
            else
            {
                auto pickleStepIter = std::ranges::find(pickle.steps, testStep.pickle_step_id.value(), &cucumber::messages::pickle_step::id);
                testStepResult = RunStep(*pickleStepIter, testStep, testCaseContext, testStepStarted);
                seenSteps = true;
            }
            testStepResults.emplace_back(testStepResult);

            broadcaster.BroadcastEvent({ .test_step_finished = cucumber::messages::test_step_finished{
                                             .test_case_started_id = currentTestCaseStartedId,
                                             .test_step_id = testStep.id,
                                             .test_step_result = testStepResult,
                                             .timestamp = support::TimestampNow(),
                                         } });
        }

        willRetry = GetWorstStepResult().status == cucumber::messages::test_step_result_status::FAILED && moreAttemptsAvailable;

        broadcaster.BroadcastEvent({ .test_case_finished = cucumber::messages::test_case_finished{
                                         .test_case_started_id = currentTestCaseStartedId,
                                         .timestamp = support::TimestampNow(),
                                         .will_be_retried = willRetry,
                                     } });

        return willRetry;
    }

    cucumber::messages::test_step_result TestCaseRunner::RunHook(const HookRegistry::Definition& hookDefinition, bool isBeforeHook, Context& testCaseContext)
    {
        if (ShouldSkipHook(isBeforeHook))
            return {
                .duration = cucumber::messages::duration{},
                .status = cucumber::messages::test_step_result_status::SKIPPED,
            };

        return InvokeStep(hookDefinition.factory(testCaseContext));
    }

    std::vector<cucumber::messages::test_step_result> TestCaseRunner::RunStepHooks(const cucumber::messages::pickle_step& pickleStep, HookType hookType, Context& testCaseContext)
    {
        auto ids = supportCodeLibrary.hookRegistry.FindIds(hookType, pickle.tags);
        std::vector<cucumber::messages::test_step_result> results;
        results.reserve(ids.size());

        for (const auto& id : ids)
        {
            const auto& definition = supportCodeLibrary.hookRegistry.GetDefinitionById(id);
            results.emplace_back(InvokeStep(definition.factory(testCaseContext)));
        }

        return results;
    }

    cucumber::messages::test_step_result TestCaseRunner::RunStep(const cucumber::messages::pickle_step& pickleStep, const cucumber::messages::test_step& testStep, Context& testCaseContext, cucumber::messages::test_step_started testStepStarted)
    {
        auto stepDefinitions = (*testStep.step_definition_ids) | std::views::transform([&](const std::string& id)
                                                                     {
                                                                         return supportCodeLibrary.stepRegistry.StepDefinitions().at(id);
                                                                     });

        if (const auto count = testStep.step_definition_ids->size(); count == 0)
        {
            broadcaster.BroadcastEvent({ .suggestion = cucumber::messages::suggestion{
                                             .id = idGenerator->next_id(),
                                             .pickle_step_id = pickleStep.id,
                                             .snippets = {},
                                         } });

            return {
                .duration = cucumber::messages::duration{},
                .status = cucumber::messages::test_step_result_status::UNDEFINED,
            };
        }
        else if (count > 1)
        {
            return {
                .duration = cucumber::messages::duration{},
                .message = "Ambiguous step definitions",
                .status = cucumber::messages::test_step_result_status::AMBIGUOUS,
            };
        }
        else if (IsSkippingSteps())
        {
            return {
                .duration = cucumber::messages::duration{},
                .status = cucumber::messages::test_step_result_status::SKIPPED,
            };
        }

        auto stepResults = RunStepHooks(pickleStep, HookType::beforeStep, testCaseContext);

        if (util::GetWorstTestStepResult(stepResults).status != cucumber::messages::test_step_result_status::FAILED)
        {
            const auto& definition = stepDefinitions.front();

            std::variant<std::vector<std::string>, std::vector<std::any>> parameters{};

            if (!testStep.step_match_arguments_lists->empty())
            {
                if (std::holds_alternative<cucumber_expression::Expression>(definition.regex))
                    parameters = BuildExpressionParameters(testStep.step_match_arguments_lists->front().step_match_arguments, supportCodeLibrary.parameterRegistry);
                else
                    parameters = BuildRegularParameters(testStep.step_match_arguments_lists->front().step_match_arguments);
            }

            const auto toOptionalTable = [](const cucumber::messages::pickle_step& pickleStep) -> std::optional<std::span<const cucumber::messages::pickle_table_row>>
            {
                if (pickleStep.argument && pickleStep.argument->data_table)
                    return pickleStep.argument->data_table->rows;
                return std::nullopt;
            };

            const auto result = InvokeStep(definition.factory(broadcaster, testCaseContext, testStepStarted, toOptionalTable(pickleStep), pickleStep.argument ? pickleStep.argument->doc_string : std::nullopt), parameters);
            stepResults.push_back(result);
        }

        const auto afterStepHookResults = RunStepHooks(pickleStep, HookType::afterStep, testCaseContext);
        stepResults.reserve(stepResults.size() + afterStepHookResults.size());
        stepResults.insert(stepResults.end(), afterStepHookResults.begin(), afterStepHookResults.end());

        auto finalStepResult = util::GetWorstTestStepResult(stepResults);

        cucumber::messages::duration finalDuration{};
        for (const auto& stepResult : stepResults)
            finalDuration += stepResult.duration;

        finalStepResult.duration = finalDuration;
        return finalStepResult;
    }

    cucumber::messages::test_step_result TestCaseRunner::InvokeStep(std::unique_ptr<Body> body, const ExecuteArgs& args)
    {
        return body->ExecuteAndCatchExceptions(args);
    }

    cucumber::messages::test_step_result TestCaseRunner::GetWorstStepResult() const
    {
        if (testStepResults.empty())
            return {
                .status = skip ? cucumber::messages::test_step_result_status::SKIPPED : cucumber::messages::test_step_result_status::PASSED,
            };

        return util::GetWorstTestStepResult(testStepResults);
    }

    bool TestCaseRunner::ShouldSkipHook(bool isBeforeHook)
    {
        return skip || (IsSkippingSteps() && isBeforeHook);
    }

    bool TestCaseRunner::IsSkippingSteps()
    {
        return GetWorstStepResult().status != cucumber::messages::test_step_result_status::PASSED;
    }
}
