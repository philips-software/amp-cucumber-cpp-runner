#include "cucumber_cpp/library/cucumber_expression/RegularExpression.hpp"
#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/TreeRegexp.hpp"
#include <optional>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    RegularExpression::RegularExpression(std::string expression, const ParameterRegistry& parameterRegistry)
        : expression{ std::move(expression) }
        , treeRegexp{ this->expression }
    {
        for (const auto& groupBuilder : treeRegexp.RootBuilder().Children())
        {
            const auto* parameterByRegexp = parameterRegistry.LookupByRegexp(std::string{ groupBuilder.Pattern() });
            if (parameterByRegexp != nullptr)
                parameters.emplace_back(*parameterByRegexp);
            else
            {

                parameters.emplace_back(
                    std::string{ "" },
                    std::vector<std::string>{ std::string{ groupBuilder.Pattern() } },
                    false,
                    false,
                    false,
                    std::source_location::current());
            }
        }
    }

    std::string_view RegularExpression::Source() const
    {
        return expression;
    }

    std::string_view RegularExpression::Pattern() const
    {
        return expression;
    }

    std::optional<std::vector<Argument>> RegularExpression::MatchToArguments(const std::string& text) const
    {
        auto group = treeRegexp.MatchToGroup(text);
        if (!group.has_value())
            return std::nullopt;

        return Argument::BuildArguments(group.value(), parameters);
    }
}
