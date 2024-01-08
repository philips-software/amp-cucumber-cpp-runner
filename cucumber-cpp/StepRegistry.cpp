
#include "cucumber-cpp/StepRegistry.hpp"
#include "cucumber-cpp/Context.hpp"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <bits/ranges_algo.h>
#include <iterator>
#include <ranges>
#include <regex.h>
#include <stdexcept>
#include <vector>

namespace cucumber_cpp
{
    namespace
    {
        std::string ToString(std::string toString)
        {
            return toString;
        }

        void SetStepParameters(nlohmann::json& json, const std::smatch& matchResults)
        {
            std::ranges::copy(matchResults
                                  // the first element is the whole matched string,
                                  // we are only interested in the actual matches
                                  | std::views::drop(1) | std::views::transform(ToString),
                std::back_inserter(json["parameters"]));
        }

        std::vector<std::string> ToVector(const std::smatch& matchResults)
        {
            auto range = matchResults | std::views::drop(1) | std::views::transform(ToString);
            return { range.begin(), range.end() };
        }

        std::string ParseCucumberExpression(std::string string)
        {
            if (!(string.starts_with('^') && string.ends_with('$')))
            {
                string = std::regex_replace(string, std::regex(R"(\(.*\))"), R"((?:$&)?)");

                string = std::regex_replace(string, std::regex(R"(\{int\})"), R"((-?\d+))");
                string = std::regex_replace(string, std::regex(R"(\{biginteger\})"), R"((-?\d+))");
                string = std::regex_replace(string, std::regex(R"(\{byte\})"), R"((-?\d+))");
                string = std::regex_replace(string, std::regex(R"(\{short\})"), R"((-?\d+))");
                string = std::regex_replace(string, std::regex(R"(\{long\})"), R"((-?\d+))");

                string = std::regex_replace(string, std::regex(R"(\{float\})"), R"((-?\d+\.\d+))");
                string = std::regex_replace(string, std::regex(R"(\{bigdecimal\})"), R"((-?\d+\.\d+))");
                string = std::regex_replace(string, std::regex(R"(\{double\})"), R"((-?\d+\.\d+))");

                string = std::regex_replace(string, std::regex(R"(\{word\})"), R"(([^\s]+))");
                string = std::regex_replace(string, std::regex(R"(\{string\})"), R"-("+([^"]+)"+)-");

                string = std::regex_replace(string, std::regex(R"(\{\})"), R"((.*))");

                string = "^" + string + "$";
            }

            return string;
        }

        auto TypeFilter(StepType stepType)
        {
            return [stepType](const StepRegistry::Entry& entry) -> bool
            {
                return entry.type == stepType || entry.type == StepType::any;
            };
        };
    }

    StepBase::StepBase(Context& context, const nlohmann::json& table)
        : context{ context }
        , table{ table }
    {}

    RegexMatch::RegexMatch(const std::regex& regex, const std::string& expression)
    {
        std::smatch smatch;
        matched = std::regex_search(expression, smatch, regex);
        matches = ToVector(smatch);
    }

    bool RegexMatch::Matched() const
    {
        return matched;
    }

    std::vector<std::string> RegexMatch::Matches() const
    {
        return matches;
    }

    StepRegex::StepRegex(const std::string& string)
        : string{ string }
        , regex{ ParseCucumberExpression(string) }
    {}

    std::unique_ptr<RegexMatch> StepRegex::Match(const std::string& expression) const
    {
        return std::make_unique<RegexMatch>(regex, expression);
    }

    std::string StepRegex::String() const
    {
        return string;
    }

    StepRegistry& StepRegistry::Instance()
    {
        static StepRegistry instance;
        return instance;
    }

    std::vector<StepMatch> StepRegistryBase::Query(StepType stepType, std::string expression) const
    {
        std::vector<StepMatch> matches;

        for (const Entry& entry : registry | std::views::filter(TypeFilter(stepType)))
        {
            if (auto match = entry.regex.Match(expression); match->Matched())
            {
                matches.emplace_back(std::move(match), entry.factory, entry.regex);
            }
        }

        if (matches.size() == 0)
        {
            throw std::out_of_range{ "Step: \"" + expression + "\" not found" };
        }

        return matches;
    }

    std::size_t StepRegistryBase::Size() const
    {
        return registry.size();
    }

    std::size_t StepRegistryBase::Size(StepType stepType) const
    {
        return std::ranges::count(registry, stepType, &Entry::type);
    }
}
