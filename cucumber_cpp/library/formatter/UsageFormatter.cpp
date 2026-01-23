#include "cucumber_cpp/library/formatter/UsageFormatter.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/step_definition_pattern_type.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/query/Query.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "fmt/base.h"
#include "fmt/format.h"
#include "fmt/ostream.h"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <numeric>
#include <ranges>
#include <string>
#include <utility>

namespace cucumber_cpp::library::formatter
{
    namespace
    {
        struct UsageMatch
        {
            cucumber::messages::duration duration;
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
                    .uri = stepDefinition.source_reference.uri.value_or("")
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
                    if (testStep->step_definition_ids->size() != 1)
                        continue;

                    const auto& pickleStep = query.FindPickleStepById(testStep->pickle_step_id.value());
                    const auto& pickle = query.FindPickleById(testCase.pickle_id);
                    const auto& step = query.FindStepBy(pickleStep);
                    const auto& stepDefinitionId = testStep->step_definition_ids->front();
                    const auto& lineage = query.FindLineageByPickle(pickle);

                    mapping.at(stepDefinitionId).matches.emplace_back(query.FindTestStepDurationByTestStepId(testStepFinished->test_step_id), step.location.line, pickleStep.text, lineage.gherkinDocument->uri.value_or(""));
                }
            }

            return mapping;
        }

        void BuildResult(const query::Query& query)
        {}

        std::map<std::string, Usage, std::less<>> GetUsage(const query::Query& query)
        {
            return BuildMapping(query);
        }
    }

    void UsageFormatter::OnEnvelope(const cucumber::messages::envelope& envelope)
    {
        if (envelope.test_run_finished.has_value())
        {
            const auto& mapping = GetUsage(query);

            std::size_t patternWidth = std::string("Pattern / Text").size();
            std::size_t durationWidth = std::string("Duration").size();
            std::size_t locationWidth = std::string("Location").size();

            for (const auto& usage : mapping | std::views::values)
            {
                patternWidth = std::max(patternWidth, usage.pattern.size());
                cucumber::messages::duration totalDuration{};
                locationWidth = std::max(locationWidth, fmt::format("{}:{}", usage.uri, usage.line).size());

                for (const auto& match : usage.matches)
                {
                    patternWidth = std::max(patternWidth, match.text.size() + 2);
                    totalDuration += match.duration;
                    locationWidth = std::max(locationWidth, fmt::format("{}:{}", match.uri, match.line).size());
                }

                const auto meanDuration = usage.matches.empty() ? std::chrono::milliseconds{ 0 } : std::chrono::milliseconds{ util::DurationToMilliseconds(totalDuration) / usage.matches.size() };
                durationWidth = std::max(durationWidth, fmt::format("{}", meanDuration).size());
            }

            fmt::println(outputStream, "┌─{:─<{}}─┬─{:─<{}}─┬─{:─<{}}─┐", "", patternWidth, "", durationWidth, "", locationWidth);
            fmt::println(outputStream, "│ {:<{}} │ {:<{}} │ {:<{}} │", "Pattern / Text", patternWidth, "Duration", durationWidth, "Location", locationWidth);

            for (const auto& usage : mapping | std::views::values)
            {
                auto durations = usage.matches | std::views::transform([](const UsageMatch& match) -> cucumber::messages::duration
                                                     {
                                                         return match.duration;
                                                     });
                const auto totalDuration = std::accumulate(durations.begin(), durations.end(), cucumber::messages::duration{}, [](const auto& lhs, const auto& rhs)
                    {
                        return lhs + rhs;
                    });

                const auto meanDuration = usage.matches.empty() ? std::chrono::milliseconds{ 0 } : std::chrono::milliseconds{ util::DurationToMilliseconds(totalDuration) / usage.matches.size() };

                fmt::println(outputStream, "├─{:─<{}}─┼─{:─<{}}─┼─{:─<{}}─┤", "", patternWidth, "", durationWidth, "", locationWidth);
                if (usage.matches.empty())
                    fmt::println(outputStream, "│ {:<{}} │ {:<{}} │ {:<{}} │", usage.pattern, patternWidth, "UNUSED", durationWidth, fmt::format("{}:{}", usage.uri, usage.line), locationWidth);
                else
                    fmt::println(outputStream, "│ {:<{}} │ {:<{}} │ {:<{}} │", usage.pattern, patternWidth, meanDuration, durationWidth, fmt::format("{}:{}", usage.uri, usage.line), locationWidth);

                for (const auto& match : usage.matches)
                    fmt::println(outputStream, "│   {:<{}} │ {:<{}} │ {:<{}} │", match.text, patternWidth - 2, util::DurationToMilliseconds(match.duration), durationWidth, fmt::format("{}:{}", match.uri, match.line), locationWidth);
            }
            fmt::println(outputStream, "└─{:─<{}}─┴─{:─<{}}─┴─{:─<{}}─┘", "", patternWidth, "", durationWidth, "", locationWidth);
        }
    }
}
