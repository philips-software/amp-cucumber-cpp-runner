#ifndef FORMATTER_SUMMARY_FORMATTER_HPP
#define FORMATTER_SUMMARY_FORMATTER_HPP

#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include <cstddef>
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

        const helper::Theme theme = helper::CreateCucumberTheme();
        const bool useStatusIcon = true;
        const std::size_t scenarioIndent = 0;
    };
}

#endif
