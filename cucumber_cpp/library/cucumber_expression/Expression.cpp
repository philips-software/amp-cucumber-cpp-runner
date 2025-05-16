
#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/Ast.hpp"
#include "cucumber_cpp/library/cucumber_expression/ExpressionParser.hpp"
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
        bool matched = std::regex_search(text, smatch, regex);
        if (!matched)
            return std::nullopt;

        std::vector<std::any> result;

        auto converterIter = converters.begin();
        auto matchIt = smatch.begin() + 1;

        while (matchIt != smatch.end() && converterIter != converters.end())
        {
            result.emplace_back(converterIter->converter({ matchIt, matchIt + converterIter->matches }));

            matchIt = std::next(matchIt, converterIter->matches);
            converterIter = std::next(converterIter);
        }

        return result;
    }

    std::string Expression::RewriteToRegex(const Node& node)
    {
        switch (node.type)
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

        throw std::runtime_error{ "Invalid node type" };
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
        std::string regex{ CreateString(node) };

        if (GetPossibleNodeWithParameters(node))
            throw 0;

        if (GetPossibleNodeWithOptionals(node))
            throw 0;

        if (AreNodesEmpty(node))
            throw 0;

        for (const auto& child : node.Children())
            regex += RewriteToRegex(child);

        return std::format(R"((?:{})?)", regex);
    }

    std::string Expression::RewriteAlternation(const Node& node)
    {
        for (const auto& child : node.Children())
        {
            if (child.Children().empty())
                throw 0;
            if (AreNodesEmpty(child))
                throw 0;
        }

        std::string regex{ CreateString(node) };
        regex += RewriteToRegex(node.Children().front());
        for (const auto& child : node.Children() | std::views::drop(1))
            regex += '|' + RewriteToRegex(child);

        return std::format(R"((?:{}))", regex);
    }

    std::string Expression::RewriteAlternative(const Node& node)
    {
        std::string regex{ CreateString(node) };

        for (const auto& child : node.Children())
            regex += RewriteToRegex(child);

        return regex;
    }

    std::string Expression::RewriteParameter(const Node& node)
    {
        auto parameter = parameterRegistry.Lookup(node.Text());
        if (parameter.regex.empty())
            throw 0;

        converters.emplace_back(0u, parameter.converter);

        std::string regex{};
        if (parameter.regex.size() == 1)
            regex = std::format(R"(({}))", parameter.regex.front());
        else
        {
            regex = { parameter.regex.front() };
            for (const auto& parameter : parameter.regex | std::views::drop(1))
                regex += R"()|(?:)" + parameter;
            regex = std::format(R"(((?:{})))", regex);
        }

        converters.back().matches += std::regex{ regex }.mark_count();
        return regex;
    }

    std::string Expression::RewriteExpression(const Node& node)
    {
        std::string regex{ CreateString(node) };

        for (const auto& child : node.Children())
            regex += RewriteToRegex(child);

        return std::format("^{}$", regex);
    }

    std::string Expression::CreateString(const Node& node) const
    {
        std::string regex{};
        regex.reserve(node.Children().size() * 10);
        return std::move(regex);
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
