#ifndef CUCUMBER_EXPRESSION_REGULAREXPRESSION_HPP
#define CUCUMBER_EXPRESSION_REGULAREXPRESSION_HPP

#include "cucumber/messages/group.hpp"
#include "cucumber/messages/step_match_arguments_list.hpp"
#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/TreeRegexp.hpp"
#include <optional>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    struct RegularExpression
    {
        explicit RegularExpression(std::string expression, ParameterRegistry& parameterRegistry)
            : expression{ std::move(expression) }
            , regex{ this->expression }
            , treeRegexp{ this->expression }
        {
            auto parameterIters = treeRegexp.RootBuilder().Children() | std::views::transform([&parameterRegistry](const GroupBuilder& groupBuilder) -> Parameter
                                                                            {
                                                                                return parameterRegistry.Lookup("");
                                                                            });

            parameters = { parameterIters.begin(), parameterIters.end() };
        }

        std::string_view Source() const
        {
            return expression;
        }

        std::string_view Pattern() const
        {
            return expression;
        }

        std::optional<std::vector<Argument>> MatchToArguments(const std::string& text) const
        {
            auto group = treeRegexp.MatchToGroup(text);
            if (!group.has_value())
                return std::nullopt;

            return Argument::BuildArguments(group.value(), parameters);
        }

    private:
        std::string expression;
        std::regex regex;

        TreeRegexp treeRegexp;
        std::vector<Parameter> parameters;
    };
}

#endif
