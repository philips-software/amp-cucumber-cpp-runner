#ifndef FORMATTER_SUMMARY_FORMATTER_HPP
#define FORMATTER_SUMMARY_FORMATTER_HPP

#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "nlohmann/json_fwd.hpp"
#include <cstddef>

namespace cucumber_cpp::library::formatter
{
    struct SummaryFormatter
        : Formatter
    {
        using Formatter::Formatter;

        constexpr static auto name = "summary";

    private:
        struct Options
        {
            explicit Options(const nlohmann::json& formatOptions);

            const bool useStatusIcon;
            const helper::Theme theme;
        };

        void OnEnvelope(const cucumber::messages::envelope& envelope) override;
        void LogSummary(const cucumber::messages::duration& testRunDuration);

        Options options{ formatOptions.contains(name) ? formatOptions.at(name) : nlohmann::json::object() };
    };
}

#endif
