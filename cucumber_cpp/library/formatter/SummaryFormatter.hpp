#ifndef FORMATTER_SUMMARY_FORMATTER_HPP
#define FORMATTER_SUMMARY_FORMATTER_HPP

#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/timestamp.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include <list>
#include <string_view>

namespace cucumber_cpp::library::formatter
{
    struct SummaryFormatter
        : Formatter
    {
        using Formatter::Formatter;

        constexpr static auto name = "summary";

    private:
        void OnEnvelope(const cucumber::messages::envelope& envelope) override;
        void LogSummary(const cucumber::messages::duration& testRunDuration);
        void LogIssues(const std::list<helper::TestCaseAttempt>& attempts, std::string_view title);

        cucumber::messages::timestamp testRunStartedAt{};
    };
}

#endif
