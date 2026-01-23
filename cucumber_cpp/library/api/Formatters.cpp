#include "cucumber_cpp/library/api/Formatters.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber_cpp/library/formatter/JunitXmlFormatter.hpp"
#include "cucumber_cpp/library/formatter/MessageFormatter.hpp"
#include "cucumber_cpp/library/formatter/PrettyFormatter.hpp"
#include "cucumber_cpp/library/formatter/SummaryFormatter.hpp"
#include "cucumber_cpp/library/formatter/UsageFormatter.hpp"
#include "cucumber_cpp/library/query/Query.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "nlohmann/json_fwd.hpp"
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <ostream>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <utility>

namespace cucumber_cpp::library::api
{
    FormatterOption::FormatterOption(std::string_view str)
    {
        const auto colon = str.find(':');
        name = str.substr(0, colon);
        output = colon == std::string::npos ? "" : str.substr(colon + 1);
    }

    Formatters::Formatters()
    {
        RegisterFormatter<formatter::PrettyFormatter>();
        RegisterFormatter<formatter::SummaryFormatter>();
        RegisterFormatter<formatter::JunitXmlFormatter>();
        RegisterFormatter<formatter::MessageFormatter>();
        RegisterFormatter<formatter::UsageFormatter>();
    }

    std::set<std::pair<std::string, bool>> Formatters::GetAvailableFormatterNames() const
    {
        auto view = availableFormatters | std::views::transform([](const auto& pair) -> std::pair<std::string, bool>
                                              {
                                                  return { pair.first, pair.second.hasOutput };
                                              });
        return { view.begin(), view.end() };
    }

    std::list<std::unique_ptr<formatter::Formatter>> Formatters::EnableFormatters(const std::set<std::string, std::less<>>& format, const nlohmann::json& formatOptions, support::SupportCodeLibrary& supportCodeLibrary, query::Query& query, std::ostream& output)
    {
        std::list<std::unique_ptr<formatter::Formatter>> activeFormatters;

        for (const auto& formatterName : format)
        {
            const FormatterOption option{ formatterName };

            if (option.output.empty())
                activeFormatters.emplace_back(availableFormatters.at(option.name).factory(supportCodeLibrary, query, formatOptions, output));
            else
            {
                const auto absolutePath = std::filesystem::absolute(std::filesystem::path{ option.output }).string();
                if (!customOutputFiles.contains(absolutePath))
                    customOutputFiles.try_emplace(absolutePath, std::make_unique<std::ofstream>(absolutePath));

                activeFormatters.emplace_back(availableFormatters.at(option.name).factory(supportCodeLibrary, query, formatOptions, *customOutputFiles.at(absolutePath)));
            }
        }

        return activeFormatters;
    }
}
