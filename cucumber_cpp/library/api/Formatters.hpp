#ifndef API_FORMATTERS_HPP
#define API_FORMATTERS_HPP

#include "cucumber_cpp/library/Query.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>

namespace cucumber_cpp::library::api
{
    struct Formatters
    {
        Formatters();

        template<class T>
        void RegisterFormatter(const std::string& name);

        std::vector<std::string> GetAvailableFormatterNames() const;

        [[nodiscard]] std::list<std::unique_ptr<formatter::Formatter>> EnableFormatters(const std::set<std::string>& format, const std::string& formatOptions, support::SupportCodeLibrary& supportCodeLibrary, Query& query, const formatter::helper::EventDataCollector& eventDataCollector, std::ostream& output = std::cout);

    private:
        std::map<std::string, std::function<std::unique_ptr<formatter::Formatter>(support::SupportCodeLibrary&, Query&, const formatter::helper::EventDataCollector&, std::ostream&)>> availableFormatters;
    };

    ////////////////////
    // Implementation //
    ////////////////////

    template<class T>
    void Formatters::RegisterFormatter(const std::string& name)
    {
        availableFormatters.try_emplace(name, [](support::SupportCodeLibrary& supportCodeLibrary, Query& query, const formatter::helper::EventDataCollector& eventDataCollector, std::ostream& output)
            {
                return std::make_unique<T>(supportCodeLibrary, query, eventDataCollector, output);
            });
    }
}

#endif
