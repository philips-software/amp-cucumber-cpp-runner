#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber/cucumber-expressions/Argument.hpp"
#include "cucumber/cucumber-expressions/Errors.hpp"
#include "cucumber/cucumber-expressions/Expression.hpp"
#include "cucumber/cucumber-expressions/Matcher.hpp"
#include "cucumber/cucumber-expressions/ParameterRegistry.hpp"
#include "cucumber/cucumber-expressions/RegularExpression.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber_cpp/library/support/DefinitionRegistration.hpp"
#include "cucumber_cpp/library/support/StepType.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/UndefinedParameters.hpp"
#include "cucumber_cpp/library/util/StepFactory.hpp"
#include <cstddef>
#include <iterator>
#include <list>
#include <map>
#include <optional>
#include <source_location>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::support
{
    StepRegistry::StepRegistry(cucumber::cucumber_expressions::ParameterRegistry& parameterRegistry, support::UndefinedParameters& undefinedParameters, cucumber::gherkin::IdGeneratorPtr idGenerator)
        : parameterRegistry{ parameterRegistry }
        , undefinedParameters{ undefinedParameters }
        , idGenerator{ std::move(idGenerator) }
    {
    }

    void StepRegistry::LoadSteps()
    {
        support::DefinitionRegistration::Instance().ForEachRegisteredStep([this](const StepStringRegistration::Entry& entry)
            {
                Register(entry.id, entry.regex, entry.type, entry.factory, entry.sourceLocation);
            });
    }

    [[nodiscard]] std::pair<std::vector<std::string>, std::vector<std::vector<cucumber::cucumber_expressions::Argument>>> StepRegistry::FindDefinitions(const std::string& expression) const
    {
        std::pair<std::vector<std::string>, std::vector<std::vector<cucumber::cucumber_expressions::Argument>>> result;
        result.first.reserve(idToDefinitionMap.size());
        result.second.reserve(idToDefinitionMap.size());

        for (const auto& [id, iter] : idToDefinitionMap)
        {
            const auto match = std::visit(cucumber::cucumber_expressions::MatchVisitor{ expression }, iter->regex);
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

    util::StepFactory StepRegistry::GetFactoryById(const std::string& id) const
    {
        return idToDefinitionMap.at(id)->factory;
    }

    const StepRegistry::Definition& StepRegistry::GetDefinitionById(const std::string& id) const
    {
        return *idToDefinitionMap.at(id);
    }

    const std::list<StepRegistry::Definition>& StepRegistry::StepDefinitions() const
    {
        return registry;
    }

    void StepRegistry::Register(std::string id, const std::string& matcher, StepType stepType, util::StepFactory factory, std::source_location sourceLocation)
    {
        try
        {
            auto cucumberMatcher = (matcher.starts_with('^') || matcher.ends_with('$'))
                                       ? cucumber::cucumber_expressions::Matcher{
                                             std::in_place_type<cucumber::cucumber_expressions::RegularExpression>,
                                             matcher,
                                             parameterRegistry,
                                         }
                                       : cucumber::cucumber_expressions::Matcher{
                                             std::in_place_type<cucumber::cucumber_expressions::Expression>,
                                             matcher,
                                             parameterRegistry,
                                         };
            auto cucumberMatcherType = std::holds_alternative<cucumber::cucumber_expressions::RegularExpression>(cucumberMatcher)
                                           ? ExpressionPatternType::regularExpression
                                           : ExpressionPatternType::cucumberExpression;

            registry.emplace_back(factory,
                id,
                sourceLocation.line(),
                sourceLocation.file_name(),
                stepType,
                matcher,
                std::move(cucumberMatcher),
                cucumberMatcherType);

            idToDefinitionMap[id] = std::prev(registry.end());
        }
        catch (const cucumber::cucumber_expressions::UndefinedParameterTypeError& e)
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
