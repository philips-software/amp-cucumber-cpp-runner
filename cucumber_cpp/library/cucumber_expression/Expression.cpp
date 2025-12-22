
#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/Ast.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/ExpressionParser.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include <algorithm>
#include <format>
#include <optional>
#include <ranges>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    Expression::Expression(std::string expression, ParameterRegistry& parameterRegistry)
        : expression{ std::move(expression) }
        , parameterRegistry{ parameterRegistry }
        , pattern{ RewriteToRegex(ExpressionParser{}.Parse(this->expression)) }
        , treeRegexp{ pattern }
    {
    }

    std::string_view Expression::Source() const
    {
        return expression;
    }

    std::string_view Expression::Pattern() const
    {
        return pattern;
    }

    std::optional<std::vector<Argument>> Expression::MatchToArguments(const std::string& text) const
    {
        auto group = treeRegexp.MatchToGroup(text);
        if (!group.has_value())
            return std::nullopt;

        return Argument::BuildArguments(group.value(), parameters);
    }

    std::string Expression::RewriteToRegex(const Node& node)
    {
        switch (node.Type())
        {
            case NodeType::text:
                return EscapeRegex(node.Text());
            case NodeType::optional:
                return RewriteOptional(node);
            case NodeType::alternation:
                return RewriteAlternation(node);
            case NodeType::alternative:
                return RewriteAlternative(node);
            case NodeType::parameter:
                return RewriteParameter(node);
            case NodeType::expression:
                return RewriteExpression(node);
        }

        throw InvalidNodeType{ "Invalid node type" };
    }

    std::string Expression::EscapeRegex(std::string_view text) const
    {
        using namespace std::literals;
        std::string escapedText{};
        escapedText.reserve(text.size() * 2);
        const auto escapePattern = R"(\^[({$.|?*+})])"sv;

        for (const auto& character : text)
        {
            if (std::ranges::find(escapePattern, character) != escapePattern.end())
                escapedText += '\\';
            escapedText += character;
        }

        return escapedText;
    }

    std::string Expression::RewriteOptional(const Node& node)
    {
        std::string partialRegex{ CreateEmptyRegexString(node) };

        if (ContainsNodeWithParameters(node))
            throw ParameterIsNotAllowedInOptional(node, expression);

        if (ContainsNodeWithOptionals(node))
            throw OptionalIsNotAllowedInOptional(node, expression);

        if (NodesAreEmpty(node))
            throw OptionalMayNotBeEmpty(node, expression);

        for (const auto& child : node.Children())
            partialRegex += RewriteToRegex(child);

        return std::format(R"((?:{})?)", partialRegex);
    }

    std::string Expression::RewriteAlternation(const Node& node)
    {
        for (const auto& child : node.Children())
        {
            if (child.Children().empty())
                throw AlternativeMayNotBeEmpty(node, expression);

            if (NodesAreEmpty(child))
                throw AlternativeMayNotExclusivelyContainOptionals(node, expression);
        }

        std::string partialRegex{ CreateEmptyRegexString(node) };
        partialRegex += RewriteToRegex(node.Children().front());
        for (const auto& child : node.Children() | std::views::drop(1))
            partialRegex += '|' + RewriteToRegex(child);

        return std::format(R"((?:{}))", partialRegex);
    }

    std::string Expression::RewriteAlternative(const Node& node)
    {
        std::string partialRegex{ CreateEmptyRegexString(node) };

        for (const auto& child : node.Children())
            partialRegex += RewriteToRegex(child);

        return partialRegex;
    }

    std::string Expression::RewriteParameter(const Node& node)
    {
        try
        {
            auto parameter = parameterRegistry.Lookup(node.Text());
            if (parameter.regex.empty())
                throw UndefinedParameterTypeError(node, expression, node.Text());

            parameters.push_back(parameter);

            std::string partialRegex{};
            if (parameter.regex.size() == 1)
                partialRegex = std::format(R"(({}))", parameter.regex.front());
            else
            {
                partialRegex = { parameter.regex.front() };
                for (const auto& parameterRegex : parameter.regex | std::views::drop(1))
                    partialRegex += R"()|(?:)" + parameterRegex;
                partialRegex = std::format(R"(((?:{})))", partialRegex);
            }
            return partialRegex;
        }
        catch (const std::out_of_range&)
        {
            throw UndefinedParameterTypeError(node, expression, node.Text());
        }
    }

    std::string Expression::RewriteExpression(const Node& node)
    {
        std::string partialRegex{ CreateEmptyRegexString(node) };

        for (const auto& child : node.Children())
            partialRegex += RewriteToRegex(child);

        return std::format("^{}$", partialRegex);
    }

    std::string Expression::CreateEmptyRegexString(const Node& node) const
    {
        std::string partialRegex{};
        partialRegex.reserve(node.Children().size() * 10);
        return partialRegex;
    }

    bool Expression::NodesAreEmpty(const Node& node) const
    {
        auto results = GetNodesWithType(node, NodeType::text);
        return results.empty();
    }

    bool Expression::ContainsNodeWithParameters(const Node& node) const
    {
        return ContainsNodeWithType(node, NodeType::parameter);
    }

    bool Expression::ContainsNodeWithOptionals(const Node& node) const
    {
        return ContainsNodeWithType(node, NodeType::optional);
    }

    bool Expression::ContainsNodeWithType(const Node& node, NodeType type) const
    {
        return !GetNodesWithType(node, type).empty();
    }
}
