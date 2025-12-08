#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/group.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/step_definition_pattern_type.hpp"
#include "cucumber/messages/step_match_arguments_list.hpp"
#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/Query.hpp"
#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/RegularExpression.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <source_location>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library
{
    StepRegistry::StepRegistry(cucumber_expression::ParameterRegistry& parameterRegistry, cucumber::gherkin::id_generator_ptr idGenerator)
        : parameterRegistry{ parameterRegistry }
        , idGenerator{ idGenerator }
    {
        for (const auto& matcher : StepStringRegistration::Instance().GetEntries())
            Register(matcher.regex, matcher.type, matcher.factory, matcher.sourceLocation);
    }

    StepMatch StepRegistry::Query(const std::string& expression)
    {
        std::vector<StepMatch> matches;

        for (auto& [id, entry] : registry)
        {
            auto match = std::visit(cucumber_expression::MatchVisitor{ expression }, entry.regex);
            if (match)
            {
                matches.emplace_back(entry.factory, *match, std::visit(cucumber_expression::PatternVisitor{}, entry.regex));
                ++entry.used;
            }
        }

        if (matches.empty())
            throw StepNotFoundError{};

        if (matches.size() > 1)
            throw AmbiguousStepError{ std::move(matches) };

        return std::move(matches.front());
    }

    [[nodiscard]] std::pair<std::vector<std::string>, std::vector<cucumber::messages::step_match_arguments_list>> StepRegistry::FindDefinitions(const std::string& expression)
    {
        std::pair<std::vector<std::string>, std::vector<cucumber::messages::step_match_arguments_list>> result;

        for (auto& [id, entry] : registry)
        {
            const auto match = std::visit(cucumber_expression::MatchVisitor{ expression }, entry.regex);
            if (match)
            {
                result.first.push_back(id);
            }
        }

        return result;
    }

    std::size_t StepRegistry::Size() const
    {
        return registry.size();
    }

    std::vector<StepRegistry::EntryView> StepRegistry::List() const
    {
        std::vector<StepRegistry::EntryView> list;

        list.reserve(registry.size());

        for (const auto& [id, entry] : registry)
            list.emplace_back(entry.regex, entry.used);

        return list;
    }

    StepFactory StepRegistry::GetFactoryById(std::string id) const
    {
        return registry.at(id).factory;
    }

    StepRegistry::Definition StepRegistry::GetDefinitionById(std::string id) const
    {
        return registry.at(id);
    }

    const std::map<std::string, StepRegistry::Definition>& StepRegistry::StepDefinitions() const
    {
        return registry;
    }

    void StepRegistry::Register(const std::string& matcher, engine::StepType stepType, StepFactory factory, std::source_location sourceLocation)
    {
        auto id = idGenerator->next_id();

        if (matcher.starts_with('^') || matcher.ends_with('$'))
        {
            registry.emplace(id, Definition{
                                     factory,
                                     "id",
                                     sourceLocation.line(),
                                     sourceLocation.file_name(),
                                     stepType,
                                     matcher,
                                     cucumber_expression::Matcher{
                                         std::in_place_type<cucumber_expression::RegularExpression>,
                                         matcher,
                                     },
                                     cucumber::messages::step_definition_pattern_type::REGULAR_EXPRESSION,
                                 });
        }
        else
        {
            registry.emplace(id, Definition{
                                     factory,
                                     "id",
                                     sourceLocation.line(),
                                     sourceLocation.file_name(),
                                     stepType,
                                     matcher,
                                     cucumber_expression::Matcher{
                                         std::in_place_type<cucumber_expression::Expression>,
                                         matcher,
                                         parameterRegistry,
                                     },
                                     cucumber::messages::step_definition_pattern_type::CUCUMBER_EXPRESSION,
                                 });
        }
    }

    StepStringRegistration& StepStringRegistration::Instance()
    {
        static StepStringRegistration instance;
        return instance;
    }

    std::span<StepStringRegistration::Entry> StepStringRegistration::GetEntries()
    {
        return registry;
    }

    std::span<const StepStringRegistration::Entry> StepStringRegistration::GetEntries() const
    {
        return registry;
    }
}
