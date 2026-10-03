#ifndef FORMATTER_EXTERNAL_FORMATTER_HPP
#define FORMATTER_EXTERNAL_FORMATTER_HPP

#include "cucumber/pretty-formatter/Formatter.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "nlohmann/json_fwd.hpp"
#include <memory>
#include <ostream>

namespace cucumber_cpp::library::formatter
{
    // Bridges a cucumber::pretty_formatter::Formatter onto the Broadcaster.
    struct ExternalFormatter
        : util::Listener
    {
        ExternalFormatter(util::Broadcaster& broadcaster, std::unique_ptr<cucumber::pretty_formatter::Formatter> formatter);

    private:
        std::unique_ptr<cucumber::pretty_formatter::Formatter> formatter;
    };

    std::unique_ptr<cucumber::pretty_formatter::Formatter> MakePrettyPrinter(const nlohmann::json& options, std::ostream& output);
    std::unique_ptr<cucumber::pretty_formatter::Formatter> MakeSummaryPrinter(const nlohmann::json& options, std::ostream& output);
    std::unique_ptr<cucumber::pretty_formatter::Formatter> MakeProgressPrinter(const nlohmann::json& options, std::ostream& output);
    std::unique_ptr<cucumber::pretty_formatter::Formatter> MakeProgressBarPrinter(const nlohmann::json& options, std::ostream& output);
}

#endif
