#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/step_definition_pattern_type.hpp"
#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/RegularExpression.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/UndefinedParameters.hpp"
#include <cstddef>
#include <iterator>
#include <list>
#include <map>
#include <optional>
#include <regex>
#include <source_location>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library
{
    StepRegistry::StepRegistry(cucumber_expression::ParameterRegistry& parameterRegistry, support::UndefinedParameters& undefinedParameters, cucumber::gherkin::id_generator_ptr idGenerator)
        : parameterRegistry{ parameterRegistry }
        , undefinedParameters{ undefinedParameters }
        , idGenerator{ idGenerator }
    {
    }

    void StepRegistry::LoadSteps()
    {
        for (const auto& matcher : support::DefinitionRegistration::Instance().GetSteps())
            Register(matcher.id, matcher.regex, matcher.type, matcher.factory, matcher.sourceLocation);
    }

    [[nodiscard]] std::pair<std::vector<std::string>, std::vector<std::vector<cucumber_expression::Argument>>> StepRegistry::FindDefinitions(const std::string& expression) const
    {
        std::pair<std::vector<std::string>, std::vector<std::vector<cucumber_expression::Argument>>> result;
        result.first.reserve(idToDefinitionMap.size());
        result.second.reserve(idToDefinitionMap.size());

        for (const auto& [id, iter] : idToDefinitionMap)
        {
            const auto match = std::visit(cucumber_expression::MatchVisitor{ expression }, iter->regex);
            if (match)
            {
                result.first.push_back(id);
                result.second.push_back(match.value());
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

        for (const auto& entry : registry)
            list.emplace_back(entry.regex, entry.used);

        return list;
    }

    StepFactory StepRegistry::GetFactoryById(const std::string& id) const
    {
        return idToDefinitionMap.at(id)->factory;
    }

    StepRegistry::Definition StepRegistry::GetDefinitionById(const std::string& id) const
    {
        return *idToDefinitionMap.at(id);
    }

    const std::list<StepRegistry::Definition>& StepRegistry::StepDefinitions() const
    {
        return registry;
    }

    void StepRegistry::Register(std::string id, const std::string& matcher, engine::StepType stepType, StepFactory factory, std::source_location sourceLocation)
    {
        try
        {
            auto cucumberMatcher = (matcher.starts_with('^') || matcher.ends_with('$'))
                                       ? cucumber_expression::Matcher{
                                             std::in_place_type<cucumber_expression::RegularExpression>,
                                             matcher,
                                             parameterRegistry,
                                         }
                                       : cucumber_expression::Matcher{
                                             std::in_place_type<cucumber_expression::Expression>,
                                             matcher,
                                             parameterRegistry,
                                         };
            auto cucumberMatcherType = std::holds_alternative<cucumber_expression::RegularExpression>(cucumberMatcher)
                                           ? cucumber::messages::step_definition_pattern_type::REGULAR_EXPRESSION
                                           : cucumber::messages::step_definition_pattern_type::CUCUMBER_EXPRESSION;

            registry.emplace_back(factory,
                id,
                sourceLocation.line(),
                sourceLocation.file_name(),
                stepType,
                matcher,
                cucumberMatcher,
                cucumberMatcherType);

            idToDefinitionMap[id] = std::prev(registry.end());
        }
        catch (const cucumber_expression::UndefinedParameterTypeError& e)
        {
            undefinedParameters.definitions.emplace_back(
                std::string{ e.expression },
                std::string{ e.undefinedParameterName });
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
