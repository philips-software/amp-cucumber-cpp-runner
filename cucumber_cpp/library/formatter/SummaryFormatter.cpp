#include "cucumber_cpp/library/formatter/SummaryFormatter.hpp"
#include "cucumber/messages/Duration.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber/messages/Scenario.hpp"
#include "cucumber/messages/TestCase.hpp"
#include "cucumber/messages/TestCaseFinished.hpp"
#include "cucumber/messages/TestCaseStarted.hpp"
#include "cucumber/messages/TestStep.hpp"
#include "cucumber/messages/TestStepFinished.hpp"
#include "cucumber/messages/TestStepResult.hpp"
#include "cucumber/messages/TestStepResultStatus.hpp"
#include "cucumber/query/Query.hpp"
#include "cucumber_cpp/library/formatter/helper/FormatMessages.hpp"
#include "cucumber_cpp/library/formatter/helper/PrintMessages.hpp"
#include "cucumber_cpp/library/formatter/helper/TextBuilder.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber_cpp/library/util/ToLower.hpp"
#include "fmt/ostream.h"
#include "fmt/ranges.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::formatter
{
    namespace
    {
        bool IsFailure(cucumber::messages::TestStepResultStatus status, bool willBeRetried)
        {
            return status == cucumber::messages::TestStepResultStatus::AMBIGUOUS ||
                   status == cucumber::messages::TestStepResultStatus::UNDEFINED ||
                   (status == cucumber::messages::TestStepResultStatus::FAILED && !willBeRetried);
        }

        bool IsWarning(cucumber::messages::TestStepResultStatus status, bool willBeRetried)
        {
            return status == cucumber::messages::TestStepResultStatus::PENDING ||
                   (status == cucumber::messages::TestStepResultStatus::FAILED && willBeRetried);
        }

        struct SummaryData
        {
            std::map<std::string, const cucumber::messages::TestCaseStarted*, std::less<>> warningTestStepResults;
            std::map<std::string, const cucumber::messages::TestCaseStarted*, std::less<>> failedTestStepResults;
            std::map<cucumber::messages::TestStepResultStatus, std::size_t, std::less<>> scenarioCounts;
            std::map<cucumber::messages::TestStepResultStatus, std::size_t, std::less<>> stepCounts;
            cucumber::messages::Duration totalStepDuration{};
        };

        void ClassifyTestCaseResult(
            const std::string& id,
            const cucumber::messages::TestCaseStarted* testCaseStarted,
            const cucumber::messages::TestStepResult* testStepResult,
            bool willBeRetried,
            std::map<std::string, const cucumber::messages::TestCaseStarted*, std::less<>>& warningResults,
            std::map<std::string, const cucumber::messages::TestCaseStarted*, std::less<>>& failedResults)
        {
            if (testStepResult == nullptr)
                return;

            if (IsWarning(testStepResult->status, willBeRetried))
                warningResults[id] = testCaseStarted;

            if (IsFailure(testStepResult->status, willBeRetried))
                failedResults[id] = testCaseStarted;
        }

        void CountScenarioResult(
            const cucumber::query::Query& query,
            const cucumber::messages::TestCaseFinished& testCaseFinished,
            std::map<cucumber::messages::TestStepResultStatus, std::size_t, std::less<>>& scenarioCounts)
        {
            if (const auto* testStepResultPtr = query.FindMostSevereTestStepResultBy(testCaseFinished); testStepResultPtr != nullptr)
                ++scenarioCounts[testStepResultPtr->status];
            else
                ++scenarioCounts[cucumber::messages::TestStepResultStatus::PASSED];
        }

        void CountStepResults(
            const cucumber::query::Query& query,
            const cucumber::messages::TestCaseStarted& testCaseStarted,
            std::map<cucumber::messages::TestStepResultStatus, std::size_t, std::less<>>& stepCounts,
            cucumber::messages::Duration& totalStepDuration)
        {
            const auto testStepFinishedAndTestStep = query.FindTestStepFinishedAndTestStepBy(testCaseStarted);
            for (const auto& [testStepFinished, testStep] : testStepFinishedAndTestStep)
            {
                if (testStep->pickleStepId.has_value())
                {
                    ++stepCounts[testStepFinished->testStepResult.status];
                    totalStepDuration += testStepFinished->testStepResult.duration;
                }
            }
        }

        SummaryData CollectSummaryData(const cucumber::query::Query& query)
        {
            SummaryData data;

            for (const auto& testCaseStarted : query.FindAllTestCaseStarted())
            {
                const auto* testCaseFinished = query.FindTestCaseFinishedBy(testCaseStarted);
                if (testCaseFinished == nullptr)
                    continue;
                const auto* testStepResult = query.FindMostSevereTestStepResultBy(testCaseStarted);

                ClassifyTestCaseResult(testCaseStarted.id, &testCaseStarted, testStepResult, testCaseFinished->willBeRetried, data.warningTestStepResults, data.failedTestStepResults);

                if (!testCaseFinished->willBeRetried)
                {
                    CountScenarioResult(query, *testCaseFinished, data.scenarioCounts);
                    CountStepResults(query, testCaseStarted, data.stepCounts, data.totalStepDuration);
                }
            }

            return data;
        }

        std::size_t CalculateLength(const cucumber::query::Query& query, const cucumber::messages::Pickle& pickle, const cucumber::messages::TestCaseStarted& testCaseStarted, const cucumber::messages::TestCaseFinished& testCaseFinished, const cucumber::messages::Scenario& scenario, const cucumber::messages::TestCase& testCase, bool useStatusIcon, const helper::Theme& theme) // NOSONAR: cohesive formatting helper
        {
            const auto scenarioLength = helper::Unstyled(helper::FormatPickleAttemptTitle(pickle, testCaseStarted.attempt, testCaseFinished.willBeRetried, scenario, theme)).length();

            const auto toLength = [&query, useStatusIcon, &theme, isBeforeHook = true](const cucumber::messages::TestStep& testStep) mutable -> std::size_t
            {
                if (testStep.hookId.has_value())
                {
                    const auto* hookPtr = query.FindHookBy(testStep);
                    if (hookPtr == nullptr)
                        return 0;
                    return helper::Unstyled(helper::FormatHookTitle(*hookPtr, cucumber::messages::TestStepResultStatus::UNKNOWN, isBeforeHook, useStatusIcon, theme)).length();
                }
                else if (testStep.pickleStepId.has_value())
                {
                    isBeforeHook = false;

                    const auto* pickleStep = query.FindPickleStepBy(testStep);
                    const auto* step = query.FindStepBy(*pickleStep);
                    return helper::Unstyled(helper::FormatStepTitle(testStep, *pickleStep, *step, cucumber::messages::TestStepResultStatus::UNKNOWN, useStatusIcon, theme)).length();
                }
                return 0;
            };

            auto steplengths = testCase.testSteps | std::views::transform(toLength);

            const auto maxStepLengthIter = std::ranges::max_element(steplengths);
            const auto maxStepLength = (maxStepLengthIter != steplengths.end()) ? *maxStepLengthIter : 0;
            const auto maxContentLength = std::max(scenarioLength, useStatusIcon ? maxStepLength + 2 : maxStepLength);

            return maxContentLength;
        }

        void HandleHookStep(std::ostream& stream, const cucumber::query::Query& query, const cucumber::messages::TestStepFinished* testStepFinished, const cucumber::messages::TestStep* testStep, std::size_t scenarioIndent, std::size_t maxContentLength, bool isBeforeHook, bool useStatusIcon, const helper::Theme& theme) // NOSONAR: cohesive formatting helper
        {
            if (const auto* hookPtr = query.FindHookBy(*testStep); hookPtr != nullptr)
                helper::PrintHookLine(stream, *testStepFinished, *hookPtr, scenarioIndent, maxContentLength, isBeforeHook, useStatusIcon, theme);

            helper::PrintError(stream, *testStepFinished, scenarioIndent, useStatusIcon, theme);
        }

        void HandleTestStep(std::ostream& stream, const cucumber::query::Query& query, const cucumber::messages::TestStepFinished* testStepFinished, const cucumber::messages::TestStep* testStep, std::size_t scenarioIndent, std::size_t maxContentLength, bool useStatusIcon, const helper::Theme& theme) // NOSONAR: cohesive formatting helper
        {
            const auto* pickleStep = query.FindPickleStepBy(*testStep);
            const auto* step = query.FindStepBy(*pickleStep);
            const auto* stepDefinition = query.FindUnambiguousStepDefinitionBy(*testStep);

            helper::PrintStepLine(stream, *testStepFinished, *testStep, *pickleStep, *step, stepDefinition, scenarioIndent, maxContentLength, useStatusIcon, theme);

            helper::PrintStepArgument(stream, *pickleStep, scenarioIndent, useStatusIcon, theme);
            helper::PrintAmbiguousStep(stream, query, *testStepFinished, *testStep, scenarioIndent, useStatusIcon, theme);

            helper::PrintError(stream, *testStepFinished, scenarioIndent, useStatusIcon, theme);
        }

        void HandleTestSteps(std::ostream& stream, const cucumber::query::Query& query, const cucumber::messages::TestCaseStarted& testCaseStarted, std::size_t scenarioIndent, std::size_t maxContentLength, bool useStatusIcon, const helper::Theme& theme)
        {
            const auto testStepFinishedAndTestStep = query.FindTestStepFinishedAndTestStepBy(testCaseStarted);

            auto isBeforeHook = true;
            for (const auto& [testStepFinished, testStep] : testStepFinishedAndTestStep)
            {
                if (testStep->hookId.has_value())
                    HandleHookStep(stream, query, testStepFinished, testStep, scenarioIndent, maxContentLength, isBeforeHook, useStatusIcon, theme);
                else
                {
                    isBeforeHook = false;
                    HandleTestStep(stream, query, testStepFinished, testStep, scenarioIndent, maxContentLength, useStatusIcon, theme);
                }
            }
        }

        void HandleTestCaseStarted(std::ostream& stream, const cucumber::query::Query& query, const cucumber::messages::TestCaseStarted& testCaseStarted, bool useStatusIcon, const helper::Theme& theme)
        {
            auto scenarioIndent = 0;

            const auto* testCaseFinished = query.FindTestCaseFinishedBy(testCaseStarted);
            const auto* pickle = query.FindPickleBy(testCaseStarted);
            const auto lineageAndPickle = query.FindLineageBy(*pickle).value();
            const auto& lineage = *lineageAndPickle.lineage;
            const auto* scenario = lineage.scenario;
            const auto* testCase = query.FindTestCaseBy(testCaseStarted);

            const auto maxContentLength = CalculateLength(query, *pickle, testCaseStarted, *testCaseFinished, *scenario, *testCase, useStatusIcon, theme);

            fmt::println(stream, "");
            helper::PrintScenarioAttemptLine(stream, *pickle, testCaseStarted.attempt, testCaseFinished->willBeRetried, *scenario, scenarioIndent, maxContentLength, theme);
            HandleTestSteps(stream, query, testCaseStarted, scenarioIndent, maxContentLength, useStatusIcon, theme);
        }

        void HandleTestCaseStartedList(std::ostream& stream, const cucumber::query::Query& query, const std::string& title, const std::map<std::string, const cucumber::messages::TestCaseStarted*, std::less<>>& testCaseStartedList, bool useStatusIcon, const helper::Theme& theme)
        {
            if (testCaseStartedList.empty())
                return;

            fmt::println(stream, "\n{}:", title);

            for (const auto& [id, testCaseStarted] : testCaseStartedList)
                HandleTestCaseStarted(stream, query, *testCaseStarted, useStatusIcon, theme);
        }

        void HandleSummary(std::ostream& stream, const std::string& summary, const std::map<cucumber::messages::TestStepResultStatus, std::size_t, std::less<>>& counts, const helper::Theme& theme)
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

    void SummaryFormatter::OnEnvelope(const cucumber::messages::Envelope& envelope)
    {
        if (envelope.testRunFinished)
        {
            const auto testRunDuration = query.FindTestRunDuration();
            LogSummary(testRunDuration.has_value() ? testRunDuration.value() : cucumber::messages::Duration{});
        }
    }

    void SummaryFormatter::LogSummary(const cucumber::messages::Duration& testRunDuration)
    {
        const auto data = CollectSummaryData(query);

        HandleTestCaseStartedList(outputStream, query, "Warnings", data.warningTestStepResults, options.useStatusIcon, options.theme);
        HandleTestCaseStartedList(outputStream, query, "Failures", data.failedTestStepResults, options.useStatusIcon, options.theme);
        HandleSummary(outputStream, "scenarios", data.scenarioCounts, options.theme);
        HandleSummary(outputStream, "steps", data.stepCounts, options.theme);

        fmt::println(outputStream, "{:%Mm %S}s (executing steps: {:%Mm %S}s)", util::DurationToMilliseconds(testRunDuration), util::DurationToMilliseconds(data.totalStepDuration));
    }
}
