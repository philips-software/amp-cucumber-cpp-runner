#include "cucumber_cpp/library/formatter/SummaryFormatter.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/formatter/helper/IssueHelpers.hpp"
#include "cucumber_cpp/library/formatter/helper/SummaryHelpers.hpp"
#include "cucumber_cpp/library/support/Polyfill.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Timestamp.hpp"
#include <cstdio>
#include <format>
#include <span>
#include <string_view>
#include <vector>

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
    }

    void SummaryFormatter::OnEnvelope(const cucumber::messages::envelope& envelope)
    {
        if (envelope.test_run_started)
        {
            testRunStartedAt = envelope.test_run_started->timestamp;
        }

        if (envelope.test_run_finished)
        {
            const auto testRunFinishedAt = envelope.test_run_finished->timestamp;
            const auto duration = testRunFinishedAt - testRunStartedAt;

            LogSummary(duration);
        }
    }

    void SummaryFormatter::LogSummary(const cucumber::messages::duration& testRunDuration)
    {
        std::vector<helper::TestCaseAttempt> failures{};
        std::vector<helper::TestCaseAttempt> warnings{};

        const auto attempts = eventDataCollector.GetTestCaseAttempts();
        for (const auto& attempt : attempts)
        {
            if (IsFailure(attempt.worstTestStepResult.status, attempt.willBeRetried))
                failures.emplace_back(attempt);

            if (IsWarning(attempt.worstTestStepResult.status, attempt.willBeRetried))
                warnings.emplace_back(attempt);
        }

        LogIssues(failures, "Failures");
        LogIssues(warnings, "Warnings");

        outputStream << helper::FormatSummary(attempts, testRunDuration);
    }

    void SummaryFormatter::LogIssues(std::span<const helper::TestCaseAttempt> attempts, std::string_view title)
    {
        if (!attempts.empty())
        {
            support::print(outputStream, "{}:\n\n", title);

            auto nr = 1;
            for (const auto& issue : attempts)
                helper::FormatIssue(outputStream, nr++, issue, supportCodeLibrary) << "\n";
        }
    }
}
