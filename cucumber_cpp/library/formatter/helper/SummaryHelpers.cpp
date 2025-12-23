#include "cucumber_cpp/library/formatter/helper/SummaryHelpers.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/formatter/GetColorFunctions.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/support/Duration.hpp"
#include "cucumber_cpp/library/support/Join.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <format>
#include <map>
#include <numeric>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::formatter::helper
{
    namespace
    {
        std::string GetCountSummary(std::span<const cucumber::messages::test_step_result> results, std::string_view type)
        {
            ColorFunctions colorFunctions{};
            std::map<cucumber::messages::test_step_result_status, std::size_t> counts;

            for (const auto& result : results)
                ++counts[result.status];

            auto values = counts | std::views::values;
            const auto total = std::accumulate(values.begin(), values.end(), std::size_t{ 0u });

            std::string text = std::format("{} {}{}", total, type, (total == 1 ? "" : "s"));

            if (total > 0)
            {
                std::vector<std::string> details;
                for (const auto& [status, count] : counts)
                {
                    auto statusStr = std::string{ cucumber::messages::to_string(status) };
                    std::transform(statusStr.begin(), statusStr.end(), statusStr.begin(), [](unsigned char c)
                        {
                            return std::tolower(c);
                        });
                    details.emplace_back(colorFunctions.ForStatus(status)(std::format("{} {}", count, statusStr)));
                }

                text += " " + support::Join(details, ", ");
            }

            return text;
        }

        std::string GetDurationSummary(const cucumber::messages::duration& duration)
        {
            const auto total = support::DurationToMilliseconds(duration);
            return std::format("{:%Mm %S}s", total);
        }
    }

    std::string FormatSummary(std::span<const TestCaseAttempt> testCaseAttempts, cucumber::messages::duration testRunDuration)
    {
        std::vector<cucumber::messages::test_step_result> testCaseResults;
        std::vector<cucumber::messages::test_step_result> testStepResults;
        cucumber::messages::duration totalStepDuration{};

        for (const auto& testCaseAttempt : testCaseAttempts)
        {
            for (const auto& [_, stepResult] : testCaseAttempt.stepResults)
                totalStepDuration += stepResult.duration;

            if (!testCaseAttempt.willBeRetried)
            {
                testCaseResults.emplace_back(testCaseAttempt.worstTestStepResult);
                for (const auto& testStep : testCaseAttempt.testCase.test_steps)
                    if (testStep.pickle_step_id)
                        testStepResults.emplace_back(testCaseAttempt.stepResults.at(testStep.id));
            }
        }

        const auto scenarioSummary = GetCountSummary(testCaseResults, "scenario");
        const auto stepSummary = GetCountSummary(testStepResults, "step");
        const auto durationSummary = std::format("{} (executing steps: {})\n", GetDurationSummary(testRunDuration), GetDurationSummary(totalStepDuration));

        return support::Join({ scenarioSummary, stepSummary, durationSummary }, "\n");
    }
}
