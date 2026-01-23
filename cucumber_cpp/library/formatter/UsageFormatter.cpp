#include "cucumber_cpp/library/formatter/UsageFormatter.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/step_definition_pattern_type.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "cucumber_cpp/library/query/Query.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "fmt/base.h"
#include "fmt/format.h"
#include "fmt/ostream.h"
#include "nlohmann/json_fwd.hpp"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <ranges>
#include <string>
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
            {
                mapping[id] = Usage{
                    .line = stepDefinition.source_reference.location.value_or(cucumber::messages::location{}).line,
                    .matches = {},
                    .pattern = stepDefinition.pattern.source,
                    .patternType = std::string{ cucumber::messages::to_string(stepDefinition.pattern.type) },
                    .uri = stepDefinition.source_reference.uri.value_or(""),
                    .meanDuration = {},
                };
            }

            return mapping;
        }

        std::map<std::string, Usage, std::less<>> BuildMapping(const query::Query& query)
        {
            auto mapping = BuildEmptyMapping(query);

            for (const auto& [testCaseStartedId, testCaseFinished] : query.TestCaseFinishedByTestCaseStartedId())
            {
                const auto& testCaseStarted = query.FindTestCaseStartedById(testCaseStartedId);
                const auto& testCase = query.FindTestCaseBy(testCaseStarted);
                const auto testStepFinishedAndTestStep = query.FindTestStepFinishedAndTestStepBy(testCaseStarted);

                for (const auto [testStepFinished, testStep] : testStepFinishedAndTestStep)
                {
                    if (!testStep->step_definition_ids.has_value() || testStep->step_definition_ids.value().size() != 1)
                        continue;

                    const auto& pickleStep = query.FindPickleStepById(testStep->pickle_step_id.value());
                    const auto& pickle = query.FindPickleById(testCase.pickle_id);
                    const auto& step = query.FindStepBy(pickleStep);
                    const auto& stepDefinitionId = testStep->step_definition_ids.value().front();
                    const auto& lineage = query.FindLineageByPickle(pickle);

                    std::optional<std::chrono::nanoseconds> duration{};
                    if (HasExecuted(testStepFinished->test_step_result.status))
                        duration = util::DurationToNanoSeconds(query.FindTestStepDurationByTestStepId(testStepFinished->test_step_id));

                    mapping.at(stepDefinitionId).matches.emplace_back(duration, step.location.line, pickleStep.text, lineage.gherkinDocument->uri.value_or(""));
                }
            }

            for (auto& usage : mapping | std::views::values)
            {

                if (usage.matches.empty())
                    usage.meanDuration = std::chrono::nanoseconds{ -1 };
                else
                {
                    std::chrono::nanoseconds totalDuration{};
                    std::size_t countDuration{};

                    for (const auto& match : usage.matches)
                    {
                        if (match.duration.has_value())
                        {
                            ++countDuration;
                            totalDuration += match.duration.value_or(std::chrono::nanoseconds{});
                        }
                    }

                    if (countDuration != 0)
                        usage.meanDuration = totalDuration / countDuration;
                }
            }

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
    }

    UsageFormatter::Options::Options(const nlohmann::json& json)
        : unusedOnly{ json.value("unused_only", false) }
        , theme{ helper::CreateTheme(json.value("theme", "cucumber")) }
    {
    }

    void UsageFormatter::OnEnvelope(const cucumber::messages::envelope& envelope)
    {
        if (envelope.test_run_finished.has_value())
        {
            const auto& mapping = GetUsage(query, options.unusedOnly);

            auto patternWidth = std::string("Pattern / Text").size();
            auto durationWidth = std::string("Duration").size();
            auto locationWidth = std::string("Location").size();

            for (const auto& usage : mapping)
            {
                patternWidth = std::max(patternWidth, usage.pattern.size());
                locationWidth = std::max(locationWidth, fmt::format("{}:{}", usage.uri, usage.line).size());

                if (usage.meanDuration.has_value())
                    durationWidth = std::max(durationWidth, fmt::format("{}", std::chrono::duration_cast<std::chrono::milliseconds>(usage.meanDuration.value())).size());
                else
                    durationWidth = std::max(durationWidth, std::string("-").size());

                for (const auto& match : usage.matches)
                {
                    patternWidth = std::max(patternWidth, match.text.size() + 2);
                    locationWidth = std::max(locationWidth, fmt::format("{}:{}", match.uri, match.line).size());
                }
            }

            fmt::println(outputStream, fmt::runtime(topRow), "", patternWidth, "", durationWidth, "", locationWidth);
            fmt::println(outputStream, "{0} {1:<{2}} {0} {3:<{4}} {0} {5:<{6}} {0}", options.theme.table.vertical, "Pattern / Text", patternWidth, "Duration", durationWidth, "Location", locationWidth);

            const auto horizontalDivider = fmt::format(fmt::runtime(middleRow), "", patternWidth, "", durationWidth, "", locationWidth);

            for (const auto& usage : mapping)
            {
                fmt::println(outputStream, "{}", horizontalDivider);

                if (usage.matches.empty())
                    fmt::println(outputStream, "{0} {1:<{2}} {0} {3:<{4}} {0} {5:<{6}} {0}", options.theme.table.vertical, usage.pattern, patternWidth, "UNUSED", durationWidth, fmt::format("{}:{}", usage.uri, usage.line), locationWidth);
                else if (usage.meanDuration.has_value())
                    fmt::println(outputStream, "{0} {1:<{2}} {0} {3:<{4}} {0} {5:<{6}} {0}", options.theme.table.vertical, usage.pattern, patternWidth, std::chrono::duration_cast<std::chrono::milliseconds>(usage.meanDuration.value()), durationWidth, fmt::format("{}:{}", usage.uri, usage.line), locationWidth);
                else
                    fmt::println(outputStream, "{0} {1:<{2}} {0} {3:<{4}} {0} {5:<{6}} {0}", options.theme.table.vertical, usage.pattern, patternWidth, "-", durationWidth, fmt::format("{}:{}", usage.uri, usage.line), locationWidth);

                for (const auto& match : usage.matches)
                    if (match.duration.has_value())
                        fmt::println(outputStream, "{0}   {1:<{2}} {0} {3:<{4}} {0} {5:<{6}} {0}", options.theme.table.vertical, match.text, patternWidth - 2, std::chrono::duration_cast<std::chrono::milliseconds>(match.duration.value()), durationWidth, fmt::format("{}:{}", match.uri, match.line), locationWidth);
                    else
                        fmt::println(outputStream, "{0}   {1:<{2}} {0} {3:<{4}} {0} {5:<{6}} {0}", options.theme.table.vertical, match.text, patternWidth - 2, "-", durationWidth, fmt::format("{}:{}", match.uri, match.line), locationWidth);
            }

            fmt::println(outputStream, fmt::runtime(bottomRow), "", patternWidth, "", durationWidth, "", locationWidth);
        }
    }
}
