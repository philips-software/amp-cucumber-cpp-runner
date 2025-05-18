#ifndef CUCUMBER_EXPRESSION_EXPRESSIONPARSER_HPP
#define CUCUMBER_EXPRESSION_EXPRESSIONPARSER_HPP

#include "cucumber_cpp/library/cucumber_expression/Ast.hpp"
#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <string_view>
#include <tuple>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    struct ExpressionParser
    {
        Node Parse(std::string_view expression);

        struct Result
        {
            std::size_t consumed;
            std::optional<Node> node;
        };

        struct ParserState
        {
            std::string_view expression;
            std::span<const Token> tokens;
            std::size_t current;
        };

        struct SubParser
        {
            std::function<Result(const ParserState& parser, const SubParser& subParser)> parser;
            std::vector<std::reference_wrapper<SubParser>> subParsers;

            Result Parse(const ParserState& parser) const;
        };

    private:
        using Parsers = std::vector<std::function<Result(ParserState)>>;
        using Tokens = std::vector<Token>;

        SubParser ParseBetweenGenerator(NodeType type, TokenType beginToken, TokenType endToken);

        std::tuple<std::size_t, std::vector<Node>> ParseTokensUntil(std::string_view expression, std::span<const std::reference_wrapper<SubParser>> parsers, std::span<const Token> tokens, std::size_t startAt, std::vector<TokenType> endTokens);
        Result ParseToken(std::string_view expression, std::span<const std::reference_wrapper<SubParser>> parsers, std::span<const Token> tokens, std::size_t startAt);

        Tokens tokens;
    };
}

#endif
