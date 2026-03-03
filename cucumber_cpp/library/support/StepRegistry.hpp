#ifndef CUCUMBER_CPP_STEPREGISTRY_HPP
#define CUCUMBER_CPP_STEPREGISTRY_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/StepType.hpp"
#include "cucumber_cpp/library/util/StepFactory.hpp"
#include <any>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <functional>
#include <list>
#include <map>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::runtime
{
    struct NestedTestCaseRunner;
}

namespace cucumber_cpp::library::support
{
    struct UndefinedParameters;
}

namespace cucumber_cpp::library::support
{

    struct StepMatch
    {
        StepMatch(util::StepFactory factory, std::variant<std::vector<std::string>, std::vector<std::any>> matches, std::string_view stepRegexStr)
            : factory(factory)
            , matches(std::move(matches))
            , stepRegexStr(stepRegexStr)
        {}

        util::StepFactory factory;
        std::variant<std::vector<std::string>, std::vector<std::any>> matches;
        std::string_view stepRegexStr;
    };

    enum class ExpressionPatternType : std::uint8_t
    {
        cucumberExpression,
        regularExpression,
    };

    struct StepRegistry
    {
        struct StepNotFoundError : std::exception
        {
            using std::exception::exception;
        };

        struct AmbiguousStepError : std::exception
        {
            explicit AmbiguousStepError(std::vector<StepMatch>&& matches)
                : matches{ std::move(matches) }
            {}

            std::vector<StepMatch> matches;
        };

        struct Definition
        {
            util::StepFactory factory;
            std::string id;
            std::size_t line;
            std::filesystem::path uri;

            StepType type;
            std::string pattern;
            cucumber_expression::Matcher regex;
            ExpressionPatternType patternType;

            std::uint32_t used{ 0 };
        };

        struct EntryView
        {
            EntryView(const cucumber_expression::Matcher& stepRegex, const std::uint32_t& used)
                : stepRegex(stepRegex)
                , used(used)
            {}

            const cucumber_expression::Matcher& stepRegex;
            const std::uint32_t& used;
        };

        explicit StepRegistry(cucumber_expression::ParameterRegistry& parameterRegistry, support::UndefinedParameters& undefinedParameters, cucumber::gherkin::id_generator_ptr idGenerator);

        void LoadSteps();

        [[nodiscard]] std::pair<std::vector<std::string>, std::vector<std::vector<cucumber_expression::Argument>>> FindDefinitions(const std::string& expression) const;

        [[nodiscard]] std::size_t Size() const;

        [[nodiscard]] util::StepFactory GetFactoryById(const std::string& id) const;
        [[nodiscard]] Definition GetDefinitionById(const std::string& id) const;

        [[nodiscard]] const std::list<Definition>& StepDefinitions() const;

    private:
        void Register(std::string id, const std::string& matcher, StepType stepType, util::StepFactory factory, std::source_location sourceLocation);

        cucumber_expression::ParameterRegistry& parameterRegistry;
        support::UndefinedParameters& undefinedParameters;
        cucumber::gherkin::id_generator_ptr idGenerator;

        std::list<Definition> registry;
        std::map<std::string, std::list<Definition>::iterator, std::less<>> idToDefinitionMap;
    };

    struct StepStringRegistration
    {
    private:
        StepStringRegistration() = default;

    public:
        static StepStringRegistration& Instance();

        struct Entry
        {
            Entry(StepType type, std::string regex, util::StepFactory factory, std::source_location sourceLocation)
                : type{ type }
                , regex{ std::move(regex) }
                , factory{ factory }
                , sourceLocation{ sourceLocation }
            {}

            StepType type{};
            std::string regex;
            util::StepFactory factory;
            std::source_location sourceLocation;
            std::string id{ "unassigned" };
        };

        template<class T>
        static std::size_t Register(const std::string& matcher, StepType stepType, std::source_location sourceLocation = std::source_location::current());

        std::span<Entry> GetEntries();
        [[nodiscard]] std::span<const Entry> GetEntries() const;

    private:
        std::vector<Entry> registry;
    };

    //////////////////////////
    //    implementation    //
    //////////////////////////

    template<class T>
    std::size_t StepStringRegistration::Register(const std::string& matcher, StepType stepType, std::source_location sourceLocation)
    {
        Instance().registry.emplace_back(stepType, matcher, util::StepBodyFactory<T>, sourceLocation);

        return Instance().registry.size();
    }
}

#endif
