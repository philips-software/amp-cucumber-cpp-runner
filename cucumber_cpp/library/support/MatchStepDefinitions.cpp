#include "cucumber_cpp/library/support/MatchStepDefinitions.hpp"
#include "cucumber/messages/Group.hpp"
#include "cucumber/messages/StepMatchArgument.hpp"
#include "cucumber/messages/StepMatchArgumentsList.hpp"
#include "cucumber_cpp/library/util/ArgumentGroupToMessageGroup.hpp"
#include <cucumber/cucumber-expressions/Argument.hpp>
#include <cucumber/cucumber-expressions/Matcher.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::support
{
    namespace
    {
        auto TransformToMatch(const std::string& text)
        {
            return [&text](const StepRegistry::Definition& definition) -> std::pair<std::string, std::optional<std::vector<cucumber::cucumber_expressions::Argument>>>
            {
                const auto match = std::visit(cucumber::cucumber_expressions::MatchVisitor{ text }, definition.regex);
                return { definition.id, match };
            };
        }

        bool HasMatch(const std::pair<std::string, std::optional<std::vector<cucumber::cucumber_expressions::Argument>>>& pair)
        {
            return pair.second.has_value();
        }
    }

    void MatchStepDefinitions(cucumber::messages::TestStep& testStep, const std::list<StepRegistry::Definition>& stepDefinitions, const std::string& text)
    {
        for (const auto& [id, match] : stepDefinitions |
                                           std::views::transform(TransformToMatch(text)) |
                                           std::views::filter(HasMatch))
        {
            testStep.stepDefinitionIds.value().push_back(id);
            auto& argumentList = testStep.stepMatchArgumentsLists.value().emplace_back();
            for (const auto& result : *match)
            {
                cucumber::messages::StepMatchArgument stepMatchArgument;
                stepMatchArgument.group = util::ArgumentGroupToMessageGroup(result.Group());
                if (!result.Name().empty())
                    stepMatchArgument.parameterTypeName = result.Name();
                argumentList.stepMatchArguments.push_back(stepMatchArgument);
            }
        }
    }
}
