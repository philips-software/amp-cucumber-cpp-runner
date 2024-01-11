#ifndef CUCUMBER_CPP_STEPREGISTRY_HPP
#define CUCUMBER_CPP_STEPREGISTRY_HPP

#include "cucumber-cpp/Body.hpp"
#include "cucumber-cpp/Context.hpp"
#include "nlohmann/json_fwd.hpp"
#include <functional>
#include <memory>
#include <ranges>
#include <regex>
#include <stdexcept>
#include <vector>

namespace cucumber_cpp
{
    enum struct StepType
    {
        given,
        when,
        then,
        any
    };

    struct StepBase
    {
        StepBase(Context& context, const nlohmann::json& table);

    protected:
        void Given(const std::string& step);
        void When(const std::string& step);
        void Then(const std::string& step);

    private:
        void Any(StepType type, const std::string& step);

    protected:
        Context& context;
        const nlohmann::json& table;
    };

    struct RegexMatch
    {
        RegexMatch(const std::regex& regex, const std::string& expression);

        bool Matched() const;
        std::vector<std::string> Matches() const;

    private:
        bool matched;
        std::vector<std::string> matches;
    };

    struct StepRegex
    {
        explicit StepRegex(const std::string& string);

        std::unique_ptr<RegexMatch> Match(const std::string& expression) const;
        std::string String() const;

    private:
        std::string string;
        std::regex regex;
    };

    struct StepMatch
    {
        std::unique_ptr<RegexMatch> regexMatch;
        std::unique_ptr<Body> (&factory)(Context& context, const nlohmann::json& table);
        const StepRegex& stepRegex;
    };

    struct StepRegistryBase
    {
        struct StepNotFound : std::out_of_range
        {
            using std::out_of_range::out_of_range;
        };

        struct Entry
        {
            StepType type;
            StepRegex regex;
            std::unique_ptr<Body> (&factory)(Context& context, const nlohmann::json& table);
        };

        std::vector<StepMatch> Query(StepType stepType, std::string expression) const;

        std::size_t Size() const;
        std::size_t Size(StepType stepType) const;

    protected:
        template<class T>
        std::size_t Register(const std::string& matcher, StepType stepType);

    private:
        template<class T>
        static std::unique_ptr<Body> Construct(Context& context, const nlohmann::json& table);

        std::vector<Entry> registry;
    };

    struct StepRegistry : StepRegistryBase
    {
    private:
        StepRegistry() = default;

    public:
        static StepRegistry& Instance();

        template<class T>
        static std::size_t Register(const std::string& matcher, StepType stepType);
    };

    //////////////////////////
    //    implementation    //
    //////////////////////////

    template<class T>
    std::size_t StepRegistryBase::Register(const std::string& matcher, StepType stepType)
    {
        registry.emplace_back(stepType, StepRegex{ matcher }, Construct<T>);
        return registry.size();
    }

    template<class T>
    std::unique_ptr<Body> StepRegistryBase::Construct(Context& context, const nlohmann::json& table)
    {
        return std::make_unique<T>(context, table);
    }

    template<class T>
    std::size_t StepRegistry::Register(const std::string& matcher, StepType stepType)
    {
        return Instance().StepRegistryBase::Register<T>(matcher, stepType);
    }
}

#endif
