#ifndef CUCUMBER_CPP_STEPREGISTRY_HPP
#define CUCUMBER_CPP_STEPREGISTRY_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/pickle_doc_string.hpp"
#include "cucumber/messages/pickle_table_row.hpp"
#include "cucumber/messages/step_definition_pattern_type.hpp"
#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <any>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library
{
    using StepFactory = std::unique_ptr<Body> (&)(util::Broadcaster& broadCaster, Context&, engine::StepOrHookStarted stepOrHookStarted, std::optional<std::span<const cucumber::messages::pickle_table_row>>, const std::optional<cucumber::messages::pickle_doc_string>&);

    template<class T>
    std::unique_ptr<Body> StepBodyFactory(util::Broadcaster& broadCaster, Context& context, engine::StepOrHookStarted stepOrHookStarted, std::optional<std::span<const cucumber::messages::pickle_table_row>> table, const std::optional<cucumber::messages::pickle_doc_string>& docString)
    {
        return std::make_unique<T>(broadCaster, context, stepOrHookStarted, table, docString);
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

        struct Definition
        {
            StepFactory factory;
            std::string id;
            std::size_t line;
            std::filesystem::path uri;

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

        void LoadSteps();

        [[nodiscard]] std::pair<std::vector<std::string>, std::vector<std::vector<cucumber_expression::Argument>>> FindDefinitions(const std::string& expression);

        [[nodiscard]] std::size_t Size() const;

        [[nodiscard]] std::vector<EntryView> List() const;

        StepFactory GetFactoryById(std::string id) const;
        Definition GetDefinitionById(std::string id) const;

        const std::list<Definition>& StepDefinitions() const;

    private:
        void Register(std::string id, const std::string& matcher, engine::StepType stepType, StepFactory factory, std::source_location sourceLocation);

        cucumber_expression::ParameterRegistry& parameterRegistry;
        cucumber::gherkin::id_generator_ptr idGenerator;

        std::list<Definition> registry;
        std::map<std::string, std::list<Definition>::iterator> idToDefinitionMap;
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
            std::string id{ "unassigned" };
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
