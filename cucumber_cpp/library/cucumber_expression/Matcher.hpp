#ifndef CUCUMBER_EXPRESSION_MATCHER_HPP
#define CUCUMBER_EXPRESSION_MATCHER_HPP

#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/RegularExpression.hpp"
#include <any>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    using Matcher = std::variant<Expression, RegularExpression>;

    struct SourceVisitor
    {
        std::string_view operator()(const auto& expression) const
        {
            return expression.Source();
        }
    };

    struct PatternVisitor
    {
        std::string_view operator()(const auto& expression) const
        {
            return expression.Pattern();
        }
    };

    struct MatchVisitor
    {
        std::optional<std::variant<std::vector<std::string>, std::vector<std::any>>> operator()(const auto& expression) const
        {
            return expression.Match(std::move(text));
        }

        const std::string& text;
    };

    static inline SourceVisitor sourceVisitor{};
    static inline PatternVisitor patternVisitor{};
    // static inline MatchVisitor matchVisitor{};
}

#endif
