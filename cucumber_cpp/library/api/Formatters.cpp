#include "cucumber_cpp/library/api/Formatters.hpp"
#include "cucumber_cpp/library/Query.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber_cpp/library/formatter/PrettyPrinter.hpp"
#include "cucumber_cpp/library/formatter/SummaryFormatter.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include <list>
#include <memory>
#include <ostream>
#include <ranges>
#include <set>
#include <string>
#include <vector>

namespace cucumber_cpp::library::api
{
    Formatters::Formatters()
    {
        RegisterFormatter<formatter::PrettyPrinter>("pretty");
        RegisterFormatter<formatter::SummaryFormatter>("summary");
    }

    std::vector<std::string> Formatters::GetAvailableFormatterNames() const
    {
        auto values = availableFormatters | std::views::keys;
        return { values.begin(), values.end() };
    }

    std::list<std::unique_ptr<formatter::Formatter>> Formatters::EnableFormatters(const std::set<std::string>& format, const std::string& formatOptions, support::SupportCodeLibrary& supportCodeLibrary, Query& query, const formatter::helper::EventDataCollector& eventDataCollector, std::ostream& output)
    {
        std::list<std::unique_ptr<formatter::Formatter>> activeFormatters;

        for (const auto& formatterName : format)
            activeFormatters.emplace_back(availableFormatters.at(formatterName)(supportCodeLibrary, query, eventDataCollector, output));

        return activeFormatters;
    }
}
