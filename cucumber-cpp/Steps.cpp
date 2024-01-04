#include "cucumber-cpp/Steps.hpp"
#include "nlohmann/json.hpp"
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <regex>
#include <span>
#include <stdexcept>

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
    }

    std::map<StepType, StepRepository::StepsVector> StepRepository::stepRepository;

    RegexMatch::RegexMatch(const std::regex regex, const std::string& expression)
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

    std::shared_ptr<RegexMatch> StepRegex::Match(const std::string& expression) const
    {
        return std::make_shared<RegexMatch>(regex, expression);
    }

    std::string StepRegex::String() const
    {
        return string;
    }

    std::size_t StepImplementation::counter;

    StepImplementation::StepImplementation()
        : id{ counter++ }
    {}

    std::size_t StepImplementation::Id() const
    {
        return id;
    }

    void StepImplementation::SetRegex(const std::string& str)
    {
        regex = std::make_unique<StepRegex>(str);
    }

    StepRegex& StepImplementation::Regex() const
    {
        return *regex;
    }

    StepRepository::StepRepository()
    {
        StepPreRegistration::RegisterAll(*this);
    }

    StepMatch StepRepository::Get(StepType stepType, std::string text) const
    {
        try
        {
            return GetByType(stepType, text);
        }
        catch (const StepNotFoundException&)
        {
            return GetByType(StepType::any, text);
        }
    }

    void StepRepository::RegisterStepImplementation(std::shared_ptr<StepImplementation> step)
    {
        step->SetRegex(step->GetText());

        stepRepository[step->GetType()].push_back(step);
    }

    std::size_t StepRepository::Size() const
    {
        return Size(StepType::given) + Size(StepType::when) + Size(StepType::then) + Size(StepType::any);
    }

    std::size_t StepRepository::Size(StepType stepType) const
    {
        if (stepRepository.contains(stepType))
        {
            return stepRepository.at(stepType).size();
        }
        else
        {
            return 0;
        }
    }

    StepMatch StepRepository::GetByType(StepType stepType, std::string text) const
    {
        try
        {
            const auto& searchVector = stepRepository.at(stepType);

            for (const auto& step : searchVector)
            {
                if (const auto match = step->Regex().Match(text); match->Matched())
                {
                    return { step, match };
                }
            }
        }
        catch (const std::out_of_range&)
        {
        }

        throw StepNotFoundException{ "Step: \"" + text + "\" not found" };
    }
}
