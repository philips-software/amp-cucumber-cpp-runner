#ifndef CUCUMBER_EXPRESSION_EXPRESSION_HPP
#define CUCUMBER_EXPRESSION_EXPRESSION_HPP

#include "cucumber_cpp/library/cucumber_expression/Ast.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include <any>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <optional>
#include <ranges>
#include <regex>
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
        std::optional<std::vector<std::any>> Match(const std::string& text) const;

    private:
        std::string RewriteToRegex(const Node& node);
        std::string EscapeRegex(std::string_view text) const;
        std::string RewriteOptional(const Node& node);
        std::string RewriteAlternation(const Node& node);
        std::string RewriteAlternative(const Node& node);
        std::string RewriteParameter(const Node& node);
        std::string RewriteExpression(const Node& node);

        std::string CreateString(const Node& node) const;

        bool AreNodesEmpty(const Node& node) const;

        std::optional<std::reference_wrapper<const Node>> GetPossibleNodeWithParameters(const Node& node) const;
        std::optional<std::reference_wrapper<const Node>> GetPossibleNodeWithOptionals(const Node& node) const;
        std::optional<std::reference_wrapper<const Node>> GetPossibleNode(const Node& node, NodeType type) const;

        auto GetNodesWithType(const Node& node, NodeType type) const
        {
            return std::views::filter(node.Children(), [type](const Node& child)
                {
                    return child.type == type;
                });
        }

        std::string expression;
        ParameterRegistry& parameterRegistry;
        std::vector<Converter> converters;
        std::string pattern;
        std::regex regex;
    };
}

#endif
