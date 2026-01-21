#include "cucumber_cpp/library/formatter/SummaryFormatter.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/formatter/helper/IssueHelpers.hpp"
#include "cucumber_cpp/library/formatter/helper/SummaryHelpers.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include "fmt/ostream.h"
#include <cstdio>
#include <list>
#include <map>
#include <span>
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
    }

    void SummaryFormatter::OnEnvelope(const cucumber::messages::envelope& envelope)
    {
        if (envelope.test_run_finished)
            LogSummary(query.FindTestRunDuration());
    }

    void SummaryFormatter::LogSummary(const cucumber::messages::duration& testRunDuration)
    {
        std::list<helper::TestCaseAttempt> failures{};
        std::list<helper::TestCaseAttempt> warnings{};

        // WIP
        // auto testCases = query.TestCaseStarted();
        // std::map<std::string, const cucumber::messages::test_case_started*> failedTestStepResults{};
        // std::map<std::string, const cucumber::messages::test_case_started*> warningTestStepResults{};

        // for (const auto& [id, testCaseStarted] : testCases)
        // {
        //     const auto& testCaseFinished = query.TestCaseFinishedByTestCaseStartedId().at(testCaseStarted.id);
        //     const auto* testStepResult = query.FindMostSevereTestStepResultBy(testCaseStarted).value_or(nullptr);

        //     if (testStepResult != nullptr && IsFailure(testStepResult->status, testCaseFinished.will_be_retried))
        //         failedTestStepResults[id] = &testCaseStarted;

        //     if (testStepResult != nullptr && IsWarning(testStepResult->status, testCaseFinished.will_be_retried))
        //         warningTestStepResults[id] = &testCaseStarted;
        // }

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

    void SummaryFormatter::LogIssues(const std::list<helper::TestCaseAttempt>& attempts, std::string_view title)
    {
        if (!attempts.empty())
        {
            fmt::print(outputStream, "{}:\n\n", title);

            auto nr = 1;
            for (const auto& issue : attempts)
                helper::FormatIssue(outputStream, nr++, issue, supportCodeLibrary) << "\n";
        }
    }
}
