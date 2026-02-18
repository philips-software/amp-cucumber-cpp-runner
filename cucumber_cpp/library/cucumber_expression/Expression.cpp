#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/Ast.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/ExpressionParser.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "fmt/format.h"
#include <algorithm>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    namespace
    {
        std::string EscapeRegex(std::string_view text)
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

        std::string CreateEmptyRegexString(const Node& node)
        {
            std::string partialRegex{};
            partialRegex.reserve(node.Children().size() * 10);
            return partialRegex;
        }

        auto GetNodesWithType(const Node& node, NodeType type)
        {
            return std::views::filter(node.Children(), [type](const Node& child)
                {
                    return child.Type() == type;
                });
        }

        bool NodesAreEmpty(const Node& node)
        {
            return GetNodesWithType(node, NodeType::text).empty();
        }

        bool ContainsNodeWithType(const Node& node, NodeType type)
        {
            return !GetNodesWithType(node, type).empty();
        }

        bool ContainsNodeWithParameters(const Node& node)
        {
            return ContainsNodeWithType(node, NodeType::parameter);
        }

        bool ContainsNodeWithOptionals(const Node& node)
        {
            return ContainsNodeWithType(node, NodeType::optional);
        }
    }

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

        return fmt::format(R"((?:{})?)", partialRegex);
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

        return fmt::format(R"((?:{}))", partialRegex);
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
                partialRegex = fmt::format(R"(({}))", parameter.regex.front());
            else
            {
                partialRegex = { parameter.regex.front() };
                for (const auto& parameterRegex : parameter.regex | std::views::drop(1))
                    partialRegex += R"()|(?:)" + parameterRegex;
                partialRegex = fmt::format(R"(((?:{})))", partialRegex);
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

        return fmt::format("^{}$", partialRegex);
    }
}
