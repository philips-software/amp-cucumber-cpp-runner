#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/Ast.hpp"
#include <cstddef>
#include <format>
#include <numeric>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::cucumber_expression
{
    std::string PointAt(std::size_t column)
    {
        return std::string(column, ' ') + "^";
    }

    std::string PointAtLocated(const auto& node)
    {
        auto pointer = PointAt(node.start);
        if (node.start + 1 < node.end)
        {
            for (auto i = node.start + 1; i < node.end; ++i)
                pointer += "-";
            pointer += "^";
        }
        return pointer;
    }

    Error::Error(std::size_t column, std::string_view expression, std::string_view pointer, std::string_view problem, std::string_view solution)
        : std::runtime_error{
            std::format(
                "This Cucumber Expression has a problem at column {}:\n"
                "\n"
                "{}\n"
                "{}\n"
                "{}\n"
                "{}\n",
                column + 1,
                expression,
                pointer,
                problem,
                solution)
        }
    {}

    CantEscape::CantEscape(std::string_view expression, std::size_t column)
        : Error{
            column,
            expression,
            PointAt(column),
            R"(Only the characters '{', '}', '(', ')', '\', '/' and whitespace can be escaped)",
            R"(If you did mean to use an '\' you can use '\\' to escape it)"
        }
    {}

    TheEndOfLineCannotBeEscaped::TheEndOfLineCannotBeEscaped(std::string_view expression)
        : Error{
            expression.length(),
            expression,
            PointAt(expression.length()),
            R"(The end of line can not be escaped)",
            R"(You can use '\\' to escape the '\')"
        }
    {}

    AlternationNotAllowedInOptional::AlternationNotAllowedInOptional(std::string_view expression, const Token& token)
        : Error{
            token.start,
            expression,
            PointAtLocated(token),
            R"(An alternation can not be used inside an optional)",
            R"(If you did not mean to use an alternation you can use '\/' to escape the '/'.
Otherwise rephrase your expression or consider using a regular expression instead.)",
        }
    {}

    InvalidParameterTypeNameInNode::InvalidParameterTypeNameInNode(std::string_view expression, const Token& token)
        : Error{
            token.start,
            expression,
            PointAtLocated(token),
            R"(Parameter names may not contain '{', '}', '(', ')', '\' or '/')",
            R"(Did you mean to use a regular expression?)",
        }
    {}

    MissingEndToken::MissingEndToken(std::string_view expression, TokenType beginToken, TokenType endToken, const Token& token)
        : Error{
            token.start,
            expression,
            PointAtLocated(token),
            std::format(R"(The '{}' does not have a matching '{}')", Token::SymbolOf(beginToken), Token::SymbolOf(endToken)),
            std::format(R"(If you did not intend to use {} you can use '\\{}' to escape the {})", Token::PurposeOf(beginToken), Token::SymbolOf(beginToken), Token::PurposeOf(beginToken)),
        }
    {}

    NoEligibleParsers::NoEligibleParsers(std::span<const Token> tokens)
        : std::runtime_error{
            std::format("No eligible parsers for [{}]", std::accumulate(tokens.begin() + 1, tokens.end(), Token::NameOf(tokens.begin()->type),
                                                            [](const auto& acc, const auto& token) -> std::string
                                                            {
                                                                return acc + ", " + Token::NameOf(token.type);
                                                            })),
        }
    {}
}
