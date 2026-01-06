#ifndef API_FORMATTERS_HPP
#define API_FORMATTERS_HPP

#include "cucumber_cpp/library/Query.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "nlohmann/json_fwd.hpp"
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>

namespace cucumber_cpp::library::api
{
    struct RegisteredFormatter
    {
        std::function<std::unique_ptr<formatter::Formatter>(support::SupportCodeLibrary&, Query&, const formatter::helper::EventDataCollector&, const nlohmann::json& formatOptions, std::ostream&)> factory;
        bool hasOutput{ false };
    };

    struct Formatters
    {
        Formatters();

        template<class T>
        void RegisterFormatter(bool hasOutput = false);

        std::set<std::pair<std::string, bool>> GetAvailableFormatterNames() const;

        [[nodiscard]] std::list<std::unique_ptr<formatter::Formatter>> EnableFormatters(const std::set<std::string, std::less<>>& format, const nlohmann::json& formatOptions, support::SupportCodeLibrary& supportCodeLibrary, Query& query, const formatter::helper::EventDataCollector& eventDataCollector, std::ostream& output = std::cout);

    private:
        std::map<std::string, RegisteredFormatter> availableFormatters;
    };

    ////////////////////
    // Implementation //
    ////////////////////

    template<class T>
    void Formatters::RegisterFormatter(bool hasOutput)
    {
        availableFormatters.try_emplace(T::name, [](support::SupportCodeLibrary& supportCodeLibrary, Query& query, const formatter::helper::EventDataCollector& eventDataCollector, const nlohmann::json& formatOptions, std::ostream& output)
            {
                return std::make_unique<T>(supportCodeLibrary, query, eventDataCollector, formatOptions, output);
            },
            hasOutput);
    }
}

#endif
