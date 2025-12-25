#ifndef CUCUMBER_EXPRESSION_EXPRESSION_HPP
#define CUCUMBER_EXPRESSION_EXPRESSION_HPP

#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/Ast.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/TreeRegexp.hpp"
#include <cctype>
#include <cstdlib>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    struct Expression
    {
        Expression(std::string expression, ParameterRegistry& parameterRegistry);

        std::string_view Source() const;
        std::string_view Pattern() const;

        std::optional<std::vector<Argument>> MatchToArguments(const std::string& text) const;

    private:
        std::string
        RewriteToRegex(const Node& node);
        std::string EscapeRegex(std::string_view text) const;
        std::string RewriteOptional(const Node& node);
        std::string RewriteAlternation(const Node& node);
        std::string RewriteAlternative(const Node& node);
        std::string RewriteParameter(const Node& node);
        std::string RewriteExpression(const Node& node);

        std::string CreateEmptyRegexString(const Node& node) const;

        bool NodesAreEmpty(const Node& node) const;

        bool ContainsNodeWithParameters(const Node& node) const;
        bool ContainsNodeWithOptionals(const Node& node) const;
        bool ContainsNodeWithType(const Node& node, NodeType type) const;

        auto GetNodesWithType(const Node& node, NodeType type) const
        {
            return std::views::filter(node.Children(), [type](const Node& child)
                {
                    return child.Type() == type;
                });
        }

        std::string expression;
        ParameterRegistry& parameterRegistry;
        std::vector<Parameter> parameters;
        std::string pattern;

        TreeRegexp treeRegexp;
    };
}

#endif
