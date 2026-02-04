#include "cucumber_cpp/library/formatter/SummaryFormatter.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/scenario.hpp"
#include "cucumber/messages/test_case.hpp"
#include "cucumber/messages/test_case_finished.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/formatter/helper/FormatMessages.hpp"
#include "cucumber_cpp/library/formatter/helper/PrintMessages.hpp"
#include "cucumber_cpp/library/formatter/helper/TextBuilder.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "cucumber_cpp/library/query/Query.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber_cpp/library/util/ToLower.hpp"
#include "fmt/ostream.h"
#include "fmt/ranges.h"
#include "nlohmann/json_fwd.hpp"
#include <algorithm>
#include <cstdio>
#include <map>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::formatter
{
    namespace
    {
        bool IsFailure(cucumber::messages::test_step_result_status status, bool willBeRetried)
        {
            return status == cucumber::messages::test_step_result_status::AMBIGUOUS ||
                   status == cucumber::messages::test_step_result_status::UNDEFINED ||
                   (status == cucumber::messages::test_step_result_status::FAILED && !willBeRetried);
        }

        bool IsWarning(cucumber::messages::test_step_result_status status, bool willBeRetried)
        {
            return status == cucumber::messages::test_step_result_status::PENDING ||
                   (status == cucumber::messages::test_step_result_status::FAILED && willBeRetried);
        }

        std::size_t CalculateLength(const query::Query& query, const cucumber::messages::pickle& pickle, const cucumber::messages::test_case_started& testCaseStarted, const cucumber::messages::test_case_finished& testCaseFinished, const cucumber::messages::scenario& scenario, const cucumber::messages::test_case& testCase, bool useStatusIcon, const helper::Theme& theme)
        {
            const auto scenarioLength = helper::Unstyled(helper::FormatPickleAttemptTitle(pickle, testCaseStarted.attempt, testCaseFinished.will_be_retried, scenario, theme)).length();

            const auto toLength = [&query, useStatusIcon, &theme, isBeforeHook = true](const cucumber::messages::test_step& testStep) mutable -> std::size_t
            {
                if (testStep.hook_id.has_value())
                {
                    const auto& hook = query.FindHookById(testStep.hook_id.value());
                    return helper::Unstyled(helper::FormatHookTitle(hook, cucumber::messages::test_step_result_status::UNKNOWN, isBeforeHook, useStatusIcon, theme)).length();
                }
                else if (testStep.hook_id.has_value())
                {
                    isBeforeHook = false;

                    const auto* pickleStep = query.FindPickleStepBy(testStep);
                    const auto& step = query.FindStepBy(*pickleStep);
                    return helper::Unstyled(helper::FormatStepTitle(testStep, *pickleStep, step, cucumber::messages::test_step_result_status::UNKNOWN, useStatusIcon, theme)).length();
                }
                return 0;
            };

            auto steplengths = testCase.test_steps | std::views::transform(toLength);

            const auto maxStepLengthIter = std::ranges::max_element(steplengths);
            const auto maxStepLength = (maxStepLengthIter != steplengths.end()) ? *maxStepLengthIter : 0;
            const auto maxContentLength = std::max(scenarioLength, useStatusIcon ? maxStepLength + 2 : maxStepLength);

            return maxContentLength;
        }

        void HandleHookStep(std::ostream& stream, const query::Query& query, const cucumber::messages::test_step_finished& testStepFinished, const cucumber::messages::test_step& testStep, std::size_t scenarioIndent, std::size_t maxContentLength, bool isBeforeHook, bool useStatusIcon, const helper::Theme& theme)
        {
            const auto& hook = query.FindHookById(testStep.hook_id.value());
            helper::PrintHookLine(stream, testStepFinished, hook, scenarioIndent, maxContentLength, isBeforeHook, useStatusIcon, theme);

            helper::PrintError(stream, testStepFinished, scenarioIndent, useStatusIcon, theme);
        }

        void HandleTestStep(std::ostream& stream, const query::Query& query, const cucumber::messages::test_step_finished& testStepFinished, const cucumber::messages::test_step& testStep, std::size_t scenarioIndent, std::size_t maxContentLength, bool useStatusIcon, const helper::Theme& theme)
        {
            const auto* pickleStep = query.FindPickleStepBy(testStep);
            const auto& step = query.FindStepBy(*pickleStep);
            const auto* stepDefinition = (testStep.step_definition_ids && !testStep.step_definition_ids->empty()) ? &query.FindStepDefinitionById(testStep.step_definition_ids->front()) : nullptr;

            helper::PrintStepLine(stream, testStepFinished, testStep, *pickleStep, step, stepDefinition, scenarioIndent, maxContentLength, useStatusIcon, theme);

            helper::PrintStepArgument(stream, *pickleStep, scenarioIndent, useStatusIcon, theme);
            helper::PrintAmbiguousStep(stream, query, testStepFinished, testStep, scenarioIndent, useStatusIcon, theme);

            helper::PrintError(stream, testStepFinished, scenarioIndent, useStatusIcon, theme);
        }

        void HandleTestSteps(std::ostream& stream, const query::Query& query, const cucumber::messages::test_case_started& testCaseStarted, std::size_t scenarioIndent, std::size_t maxContentLength, bool useStatusIcon, const helper::Theme& theme)
        {
            const auto& testStepFinishedAndTestStep = query.FindTestStepFinishedAndTestStepBy(testCaseStarted);

            auto isBeforeHook = true;
            for (const auto [testStepFinished, testStep] : testStepFinishedAndTestStep)
            {
                if (testStep->hook_id.has_value())
                    HandleHookStep(stream, query, *testStepFinished, *testStep, scenarioIndent, maxContentLength, isBeforeHook, useStatusIcon, theme);
                else
                {
                    isBeforeHook = false;
                    HandleTestStep(stream, query, *testStepFinished, *testStep, scenarioIndent, maxContentLength, useStatusIcon, theme);
                }
            }
        }

        void HandleTestCaseStarted(std::ostream& stream, const query::Query& query, const cucumber::messages::test_case_started& testCaseStarted, bool useStatusIcon, const helper::Theme& theme)
        {
            auto scenarioIndent = 0;

            const auto& testCaseFinished = query.TestCaseFinishedByTestCaseStartedId().at(testCaseStarted.id);
            const auto& pickle = query.FindPickleBy(testCaseStarted);
            const auto& lineage = query.FindLineageByPickle(pickle);
            const auto& scenario = lineage.scenario;
            const auto& rule = lineage.rule;
            const auto& feature = lineage.feature;
            const auto& testCase = query.FindTestCaseBy(testCaseStarted);

            const auto maxContentLength = CalculateLength(query, pickle, testCaseStarted, testCaseFinished, *scenario, testCase, useStatusIcon, theme);

            fmt::println(stream, "");
            helper::PrintScenarioAttemptLine(stream, pickle, testCaseStarted.attempt, testCaseFinished.will_be_retried, *scenario, scenarioIndent, maxContentLength, theme);
            HandleTestSteps(stream, query, testCaseStarted, scenarioIndent, maxContentLength, useStatusIcon, theme);
        }

        void HandleTestCaseStartedList(std::ostream& stream, const query::Query& query, const std::string& title, const std::map<std::string, const cucumber::messages::test_case_started*> testCaseStartedList, bool useStatusIcon, const helper::Theme& theme)
        {
            if (testCaseStartedList.empty())
                return;

            fmt::println(stream, "\n{}:", title);

            for (const auto& [id, testCaseStarted] : testCaseStartedList)
                HandleTestCaseStarted(stream, query, *testCaseStarted, useStatusIcon, theme);
        }

        void HandleSummary(std::ostream& stream, const std::string& summary, const std::map<cucumber::messages::test_step_result_status, std::size_t>& counts, const helper::Theme& theme)
        {
            const auto countToStatusString = [&theme](const auto& pair)
            {
                const auto& [status, count] = pair;
                return helper::TextBuilder{}
                    .Append(std::to_string(count))
                    .Space()
                    .Append(util::ToLower(std::string{ cucumber::messages::to_string(status) }))
                    .Build(theme.status.All(status));
            };

            auto countsValues = counts | std::views::values;
            const auto totalCount = std::accumulate(countsValues.begin(), countsValues.end(), std::size_t{ 0u });

            fmt::println(stream, "{} {} {}", totalCount, summary,
                fmt::join(counts | std::views::transform(countToStatusString), ", "));
        }
    }

    SummaryFormatter::Options::Options(const nlohmann::json& formatOptions)
        : useStatusIcon{ formatOptions.value("use_status_icon", true) }
        , theme{ helper::CreateTheme(formatOptions.value("theme", std::string_view{ "cucumber" })) }
    {
    }

    void SummaryFormatter::OnEnvelope(const cucumber::messages::envelope& envelope)
    {
        if (envelope.test_run_finished)
            LogSummary(query.FindTestRunDuration());
    }

    void SummaryFormatter::LogSummary(const cucumber::messages::duration& testRunDuration)
    {
        std::map<std::string, const cucumber::messages::test_case_started*> warningTestStepResults{};
        std::map<std::string, const cucumber::messages::test_case_started*> failedTestStepResults{};

        std::map<cucumber::messages::test_step_result_status, std::size_t> scenarioCounts;
        std::map<cucumber::messages::test_step_result_status, std::size_t> stepCounts;

        cucumber::messages::duration totalStepDuration{};

        for (const auto& [id, testCaseStarted] : query.TestCaseStarted())
        {
            const auto& testCaseFinished = query.TestCaseFinishedByTestCaseStartedId().at(testCaseStarted.id);
            const auto* testStepResult = query.FindMostSevereTestStepResultBy(testCaseStarted).value_or(nullptr);

            if (testStepResult != nullptr && IsWarning(testStepResult->status, testCaseFinished.will_be_retried))
                warningTestStepResults[id] = &testCaseStarted;

            if (testStepResult != nullptr && IsFailure(testStepResult->status, testCaseFinished.will_be_retried))
                failedTestStepResults[id] = &testCaseStarted;

            if (!testCaseFinished.will_be_retried)
            {
                if (const auto& testStepResultPtr = query.FindMostSevereTestStepResultBy(testCaseFinished); testStepResultPtr.has_value())
                    ++scenarioCounts[testStepResultPtr.value()->status];
                else
                    ++scenarioCounts[cucumber::messages::test_step_result_status::PASSED];

                const auto& testStepFinishedAndTestStep = query.FindTestStepFinishedAndTestStepBy(testCaseStarted);
                for (const auto& [testStepFinished, testStep] : testStepFinishedAndTestStep)
                {
                    if (testStep->pickle_step_id.has_value())
                    {
                        ++stepCounts[testStepFinished->test_step_result.status];
                        totalStepDuration += testStepFinished->test_step_result.duration;
                    }
                }
            }
        }

        HandleTestCaseStartedList(outputStream, query, "Warnings", warningTestStepResults, options.useStatusIcon, options.theme);
        HandleTestCaseStartedList(outputStream, query, "Failures", failedTestStepResults, options.useStatusIcon, options.theme);
        HandleSummary(outputStream, "scenarios", scenarioCounts, options.theme);
        HandleSummary(outputStream, "steps", stepCounts, options.theme);

        fmt::println(outputStream, "{:%Mm %S}s (executing steps: {:%Mm %S}s)", util::DurationToMilliseconds(testRunDuration), util::DurationToMilliseconds(totalStepDuration));
    }
}
