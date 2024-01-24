
#include "cucumber-cpp/StepRegistry.hpp"
#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/StepRunner.hpp"
#include <algorithm>
#include <iterator>
#include <ranges>
#include <regex>
#include <source_location>
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
            return [stepType](const StepRegistry::Entry& entry)
            {
                return entry.type == stepType || entry.type == StepType::any;
            };
        };
    }

    Step::Step(Context& context, const Table& table)
        : context{ context }
        , table{ table }
    {}

    void Step::Given(const std::string& step)
    {
        Any(StepType::given, step);
    }

    void Step::When(const std::string& step)
    {
        Any(StepType::when, step);
    }

    void Step::Then(const std::string& step)
    {
        Any(StepType::then, step);
    }

    void Step::Any(StepType type, const std::string& step)
    {
        const auto stepMatch = StepRegistry::Instance().Query(type, step);
        stepMatch.factory(context, {})->Execute(stepMatch.regexMatch->Matches());
    }

    void Step::Pending(const std::string& message, std::source_location current) const noexcept(false)
    {
        throw StepPending{ message, current };
    }

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

    StepMatch StepRegistryBase::Query(StepType stepType, const std::string& expression) const
    {
        std::vector<StepMatch> matches;

        for (const Entry& entry : registry | std::views::filter(TypeFilter(stepType)))
            if (auto match = entry.regex.Match(expression); match->Matched())
                matches.emplace_back(std::move(match), entry.factory, entry.regex);

        if (matches.empty())
            throw StepNotFoundError{};

        if (matches.size() > 1)
            throw AmbiguousStepError{ std::move(matches) };

        return std::move(matches.front());
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
