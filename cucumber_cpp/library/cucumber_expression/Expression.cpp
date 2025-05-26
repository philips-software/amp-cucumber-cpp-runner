
#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/Ast.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/ExpressionParser.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include <algorithm>
#include <any>
#include <format>
#include <functional>
#include <iterator>
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
        , regex{ pattern }
    {}

    std::string_view Expression::Source() const
    {
        return expression;
    }

    std::string_view Expression::Pattern() const
    {
        return pattern;
    }

    std::optional<std::vector<std::any>> Expression::Match(const std::string& text) const
    {
        std::smatch smatch;
        if (!std::regex_search(text, smatch, regex))
            return std::nullopt;

        std::vector<std::any> result;
        result.reserve(converters.size());

        auto converterIter = converters.begin();
        auto matchIter = smatch.begin() + 1;

        while (matchIter != smatch.end() && converterIter != converters.end())
        {
            result.emplace_back(converterIter->converter({ matchIter, matchIter + converterIter->matches }));

            matchIter = std::next(matchIter, converterIter->matches);
            converterIter = std::next(converterIter);
        }

        return result;
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
        std::string partialRegex{ CreateString(node) };

        if (GetPossibleNodeWithParameters(node))
            throw ParameterIsNotAllowedInOptional(node, expression);

        if (GetPossibleNodeWithOptionals(node))
            throw OptionalIsNotAllowedInOptional(node, expression);

        if (AreNodesEmpty(node))
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

            if (AreNodesEmpty(child))
                throw AlternativeMayNotExclusivelyContainOptionals(node, expression);
        }

        std::string partialRegex{ CreateString(node) };
        partialRegex += RewriteToRegex(node.Children().front());
        for (const auto& child : node.Children() | std::views::drop(1))
            partialRegex += '|' + RewriteToRegex(child);

        return std::format(R"((?:{}))", partialRegex);
    }

    std::string Expression::RewriteAlternative(const Node& node)
    {
        std::string partialRegex{ CreateString(node) };

        for (const auto& child : node.Children())
            partialRegex += RewriteToRegex(child);

        return partialRegex;
    }

    std::string Expression::RewriteParameter(const Node& node)
    {
        auto parameter = parameterRegistry.Lookup(node.Text());
        if (parameter.regex.empty())
            throw UndefinedParameterTypeError(node, expression, node.Text());

        converters.emplace_back(0u, parameter.converter);

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

        converters.back().matches += std::regex{ partialRegex }.mark_count();
        return partialRegex;
    }

    std::string Expression::RewriteExpression(const Node& node)
    {
        std::string partialRegex{ CreateString(node) };

        for (const auto& child : node.Children())
            partialRegex += RewriteToRegex(child);

        return std::format("^{}$", partialRegex);
    }

    std::string Expression::CreateString(const Node& node) const
    {
        std::string partialRegex{};
        partialRegex.reserve(node.Children().size() * 10);
        return partialRegex;
    }

    bool Expression::AreNodesEmpty(const Node& node) const
    {
        auto results = GetNodesWithType(node, NodeType::text);
        return results.empty();
    }

    std::optional<std::reference_wrapper<const Node>> Expression::GetPossibleNodeWithParameters(const Node& node) const
    {
        return GetPossibleNode(node, NodeType::parameter);
    }

    std::optional<std::reference_wrapper<const Node>> Expression::GetPossibleNodeWithOptionals(const Node& node) const
    {
        return GetPossibleNode(node, NodeType::optional);
    }

    std::optional<std::reference_wrapper<const Node>> Expression::GetPossibleNode(const Node& node, NodeType type) const
    {
        auto results = GetNodesWithType(node, type);
        if (results.empty())
            return std::nullopt;
        return std::ref(results.front());
    }
}
