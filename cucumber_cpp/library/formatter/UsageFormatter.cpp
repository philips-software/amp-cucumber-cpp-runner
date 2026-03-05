#include "cucumber_cpp/library/formatter/UsageFormatter.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/step_definition_pattern_type.hpp"
#include "cucumber/messages/test_case.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "cucumber_cpp/library/query/Query.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "fmt/base.h"
#include "fmt/format.h"
#include "fmt/ostream.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <numeric>
#include <optional>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

namespace cucumber_cpp::library::formatter
{
    namespace
    {
        bool HasExecuted(cucumber::messages::test_step_result_status status)
        {
            return status == cucumber::messages::test_step_result_status::UNKNOWN ||
                   status == cucumber::messages::test_step_result_status::PASSED ||
                   status == cucumber::messages::test_step_result_status::PENDING ||
                   status == cucumber::messages::test_step_result_status::FAILED;
        }

        struct UsageMatch
        {
            std::optional<std::chrono::nanoseconds> duration;
            std::size_t line;
            std::string text;
            std::string uri;
        };

        struct Usage
        {
            std::size_t line;
            std::list<UsageMatch> matches;
            std::string pattern;
            std::string patternType;
            std::string uri;
            std::optional<std::chrono::nanoseconds> meanDuration;
        };

        std::map<std::string, Usage, std::less<>> BuildEmptyMapping(const query::Query& query)
        {
            std::map<std::string, Usage, std::less<>> mapping;

            for (const auto& [id, stepDefinition] : query.StepDefinitions())
                mapping[id] = Usage{
                    .line = stepDefinition.source_reference.location.value_or(cucumber::messages::location{}).line,
                    .matches = {},
                    .pattern = stepDefinition.pattern.source,
                    .patternType = std::string{ cucumber::messages::to_string(stepDefinition.pattern.type) },
                    .uri = stepDefinition.source_reference.uri.value_or(""),
                    .meanDuration = {},
                };

            return mapping;
        }

        bool ShouldProcessTestStep(const cucumber::messages::test_step& testStep)
        {
            return testStep.step_definition_ids.has_value() && testStep.step_definition_ids.value().size() == 1;
        }

        UsageMatch CreateUsageMatch(
            const query::Query& query,
            const cucumber::messages::test_step_finished& testStepFinished,
            const cucumber::messages::test_step& testStep,
            const cucumber::messages::test_case& testCase)
        {
            const auto& pickleStep = query.FindPickleStepById(testStep.pickle_step_id.value());
            const auto& pickle = query.FindPickleById(testCase.pickle_id);
            const auto& step = query.FindStepBy(pickleStep);
            const auto& lineage = query.FindLineageByPickle(pickle);

            std::optional<std::chrono::nanoseconds> duration{};
            if (HasExecuted(testStepFinished.test_step_result.status))
                duration = util::DurationToNanoSeconds(query.FindTestStepDurationByTestStepId(testStepFinished.test_step_id));

            return UsageMatch{
                .duration = duration,
                .line = step.location.line,
                .text = pickleStep.text,
                .uri = lineage.gherkinDocument->uri.value_or("")
            };
        }

        void AddUsageMatchToMapping(
            const query::Query& query,
            const cucumber::messages::test_step_finished& testStepFinished,
            const cucumber::messages::test_step& testStep,
            const cucumber::messages::test_case& testCase,
            std::map<std::string, Usage, std::less<>>& mapping)
        {
            if (!ShouldProcessTestStep(testStep))
                return;

            const auto& stepDefinitionId = testStep.step_definition_ids.value().front();
            mapping.at(stepDefinitionId).matches.emplace_back(CreateUsageMatch(query, testStepFinished, testStep, testCase));
        }

        std::optional<std::chrono::nanoseconds> CalculateMeanDuration(const std::list<UsageMatch>& matches)
        {
            if (matches.empty())
                return std::chrono::nanoseconds{ -1 };

            auto validDurations = matches |
                                  std::views::filter([](const UsageMatch& m)
                                      {
                                          return m.duration.has_value();
                                      }) |
                                  std::views::transform([](const UsageMatch& m)
                                      {
                                          return m.duration.value();
                                      });

            const auto count = std::ranges::distance(validDurations);

            if (count == 0)
                return std::nullopt;

            const auto total = std::accumulate(validDurations.begin(), validDurations.end(), std::chrono::nanoseconds{});
            return total / count;
        }

        void PopulateMappingWithMatches(const query::Query& query, std::map<std::string, Usage, std::less<>>& mapping)
        {
            for (const auto& [testCaseStartedId, testCaseFinished] : query.TestCaseFinishedByTestCaseStartedId())
            {
                const auto& testCaseStarted = query.FindTestCaseStartedById(testCaseStartedId);
                const auto& testCase = query.FindTestCaseBy(testCaseStarted);
                const auto testStepFinishedAndTestStep = query.FindTestStepFinishedAndTestStepBy(testCaseStarted);

                for (const auto [testStepFinished, testStep] : testStepFinishedAndTestStep)
                    AddUsageMatchToMapping(query, *testStepFinished, *testStep, testCase, mapping);
            }
        }

        void FinalizeMappingWithMeanDurations(std::map<std::string, Usage, std::less<>>& mapping)
        {
            for (auto& usage : mapping | std::views::values)
                usage.meanDuration = CalculateMeanDuration(usage.matches);
        }

        std::map<std::string, Usage, std::less<>> BuildMapping(const query::Query& query)
        {
            auto mapping = BuildEmptyMapping(query);
            PopulateMappingWithMatches(query, mapping);
            FinalizeMappingWithMeanDurations(mapping);
            return mapping;
        }

        std::list<Usage> GetUsage(const query::Query& query, bool unusedOnly)
        {
            const auto& mapping = BuildMapping(query);
            auto mapValues = mapping | std::views::values | (std::views::filter([unusedOnly](const Usage& usage)
                                                                {
                                                                    return !unusedOnly || usage.matches.empty();
                                                                }));

            std::list<Usage> usageList{ mapValues.begin(), mapValues.end() };

            usageList.sort([](const Usage& lhs, const Usage& rhs)
                {
                    if (lhs.matches.empty() && !rhs.matches.empty())
                        return false;
                    if (!lhs.matches.empty() && rhs.matches.empty())
                        return true;

                    if (lhs.meanDuration == rhs.meanDuration)
                        return std::make_tuple(lhs.uri, lhs.line) < std::make_tuple(rhs.uri, rhs.line);

                    return lhs.meanDuration > rhs.meanDuration;
                });

            for (auto& usage : usageList)
                usage.matches.sort([](const UsageMatch& lhs, const UsageMatch& rhs)
                    {
                        if (lhs.duration == rhs.duration)
                            return std::make_tuple(lhs.uri, lhs.line, lhs.text) < std::make_tuple(rhs.uri, rhs.line, rhs.text);

                        return lhs.duration > rhs.duration;
                    });

            return usageList;
        }

        struct ColumnWidths
        {
            std::size_t pattern;
            std::size_t duration;
            std::size_t location;
        };

        std::string FormatLocation(const std::string& uri, std::size_t line)
        {
            return fmt::format("{}:{}", uri, line);
        }

        std::string FormatLocation(std::string_view base, const std::string& uri, std::size_t line)
        {
            return fmt::format("{}:{}", std::filesystem::relative(uri, base).make_preferred().string(), line);
        }

        std::string FormatDuration(const std::optional<std::chrono::nanoseconds>& duration)
        {
            if (duration.has_value())
                return fmt::format("{}", std::chrono::duration_cast<std::chrono::milliseconds>(duration.value()));
            return "-";
        }

        ColumnWidths CalculateColumnWidths(const std::list<Usage>& usageList)
        {
            ColumnWidths widths{
                .pattern = std::string("Pattern / Text").size(),
                .duration = std::string("Duration").size(),
                .location = std::string("Location").size()
            };

            for (const auto& usage : usageList)
            {
                widths.pattern = std::max(widths.pattern, usage.pattern.size());
                widths.location = std::max(widths.location, FormatLocation(usage.uri, usage.line).size());
                widths.duration = std::max(widths.duration, FormatDuration(usage.meanDuration).size());

                for (const auto& match : usage.matches)
                {
                    widths.pattern = std::max(widths.pattern, match.text.size() + 2);
                    widths.location = std::max(widths.location, FormatLocation(match.uri, match.line).size());
                }
            }

            return widths;
        }

        void PrintRow(std::ostream& stream, const ColumnWidths& widths, const auto& col1, const auto& col2, const auto& col3, const helper::Theme& theme)
        {
            fmt::println(stream, "{0} {1:<{2}} {0} {3:<{4}} {0} {5:<{6}} {0}", theme.table.vertical, col1, widths.pattern, col2, widths.duration, col3, widths.location);
        }

        void PrintTableHeader(std::ostream& stream, const ColumnWidths& widths, const std::string& topRow, const helper::Theme& theme)
        {
            fmt::println(stream, fmt::runtime(topRow), "", widths.pattern, "", widths.duration, "", widths.location);

            PrintRow(stream, widths, "Pattern / Text", "Duration", "Location", theme);
        }

        void PrintUsagePattern(std::ostream& stream, const Usage& usage, const ColumnWidths& widths, const helper::Theme& theme)
        {
            if (usage.matches.empty())
                PrintRow(stream, widths, usage.pattern, "UNUSED", FormatLocation(SOURCE_DIR, usage.uri, usage.line), theme);
            else
                PrintRow(stream, widths, usage.pattern, FormatDuration(usage.meanDuration), FormatLocation(SOURCE_DIR, usage.uri, usage.line), theme);
        }

        void PrintUsageMatch(std::ostream& stream, const UsageMatch& match, const ColumnWidths& widths, const helper::Theme& theme)
        {
            fmt::println(stream, "{0}   {1:<{2}} {0} {3:<{4}} {0} {5:<{6}} {0}", theme.table.vertical, match.text, widths.pattern - 2, FormatDuration(match.duration), widths.duration, FormatLocation(match.uri, match.line), widths.location);
        }

        void PrintUsageWithMatches(std::ostream& stream, const Usage& usage, const ColumnWidths& widths, const std::string& divider, const helper::Theme& theme)
        {
            fmt::println(stream, "{}", divider);
            PrintUsagePattern(stream, usage, widths, theme);

            for (const auto& match : usage.matches)
                PrintUsageMatch(stream, match, widths, theme);
        }

        void PrintUsageTable(std::ostream& stream, const std::list<Usage>& usageList, const std::string& topRow, const std::string& middleRow, const std::string& bottomRow, const helper::Theme& theme)
        {
            const auto widths = CalculateColumnWidths(usageList);
            const auto divider = fmt::format(fmt::runtime(middleRow), "", widths.pattern, "", widths.duration, "", widths.location);

            PrintTableHeader(stream, widths, topRow, theme);

            for (const auto& usage : usageList)
                PrintUsageWithMatches(stream, usage, widths, divider, theme);

            fmt::println(stream, fmt::runtime(bottomRow), "", widths.pattern, "", widths.duration, "", widths.location);
        }
    }

    UsageFormatter::Options::Options(const nlohmann::json& json)
        : unusedOnly{ json.value("unused_only", false) }
        , theme{ helper::CreateTheme(json.value("theme", "cucumber")) }
    {
    }

    void UsageFormatter::OnEnvelope(const cucumber::messages::envelope& envelope)
    {
        if (envelope.test_run_finished.has_value())
            PrintUsageTable(outputStream, GetUsage(query, options.unusedOnly), topRow, middleRow, bottomRow, options.theme);
    }
}
