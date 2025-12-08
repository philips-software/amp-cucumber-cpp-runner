#ifndef CUCUMBER_CPP_STEPREGISTRY_HPP
#define CUCUMBER_CPP_STEPREGISTRY_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/step_definition_pattern_type.hpp"
#include "cucumber/messages/step_match_arguments_list.hpp"
#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/Query.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include <any>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <map>
#include <memory>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library
{

    using StepFactory = std::unique_ptr<Body> (&)(Context&, const engine::Table&, const std::string&);

    template<class T>
    std::unique_ptr<Body> StepBodyFactory(Context& context, const engine::Table& table, const std::string& docString)
    {
        return std::make_unique<T>(context, table, docString);
    }

    struct StepMatch
    {
        StepMatch(StepFactory factory, std::variant<std::vector<std::string>, std::vector<std::any>> matches, std::string_view stepRegexStr)
            : factory(factory)
            , matches(std::move(matches))
            , stepRegexStr(stepRegexStr)
        {}

        StepFactory factory;
        std::variant<std::vector<std::string>, std::vector<std::any>> matches{};
        std::string_view stepRegexStr{};
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

        struct StepDefinition
        {
            StepFactory factory;
            std::string id;
            std::size_t line;
            std::filesystem::path uri;
        };

        struct Definition : StepDefinition
        {
            engine::StepType type;
            std::string pattern;
            cucumber_expression::Matcher regex;
            cucumber::messages::step_definition_pattern_type patternType;

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

        explicit StepRegistry(cucumber_expression::ParameterRegistry& parameterRegistry, cucumber::gherkin::id_generator_ptr idGenerator);

        [[nodiscard]] StepMatch Query(const std::string& expression);
        [[nodiscard]] std::pair<std::vector<std::string>, std::vector<cucumber::messages::step_match_arguments_list>> FindDefinitions(const std::string& expression);

        [[nodiscard]] std::size_t Size() const;

        [[nodiscard]] std::vector<EntryView> List() const;

        StepFactory GetFactoryById(std::string id) const;
        Definition GetDefinitionById(std::string id) const;

        const std::map<std::string, Definition>& StepDefinitions() const;

    private:
        void Register(const std::string& matcher, engine::StepType stepType, StepFactory factory, std::source_location sourceLocation);

        cucumber_expression::ParameterRegistry& parameterRegistry;
        cucumber::gherkin::id_generator_ptr idGenerator;

        std::map<std::string, Definition> registry;
    };

    struct StepStringRegistration
    {
    private:
        StepStringRegistration() = default;

    public:
        static StepStringRegistration& Instance();

        struct Entry
        {
            Entry(engine::StepType type, std::string regex, StepFactory factory, std::source_location sourceLocation)
                : type{ type }
                , regex{ std::move(regex) }
                , factory{ factory }
                , sourceLocation{ sourceLocation }
            {}

            engine::StepType type{};
            std::string regex;
            StepFactory factory;
            std::source_location sourceLocation;
        };

        template<class T>
        static std::size_t Register(const std::string& matcher, engine::StepType stepType, std::source_location sourceLocation = std::source_location::current());

        std::span<Entry> GetEntries();
        [[nodiscard]] std::span<const Entry> GetEntries() const;

    private:
        std::vector<Entry> registry;
    };

    //////////////////////////
    //    implementation    //
    //////////////////////////

    template<class T>
    std::size_t StepStringRegistration::Register(const std::string& matcher, engine::StepType stepType, std::source_location sourceLocation)
    {
        Instance().registry.emplace_back(stepType, matcher, StepBodyFactory<T>, sourceLocation);

        return Instance().registry.size();
    }
}

#endif
