#include "cucumber_cpp/library/cucumber_expression/ExpressionParser.hpp"
#include "cucumber_cpp/library/cucumber_expression/Ast.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/ExpressionTokenizer.hpp"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <string_view>
#include <tuple>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    namespace
    {

        template<class T>
        struct InHelper
        {
            T t;

            bool in(auto... args)
            {
                return (... || (args == t));
            }
        };

        auto MatchToken(TokenType expected)
        {
            return InHelper{ expected };
        }

        bool LookingAt(std::span<const Token> tokens, std::size_t position, TokenType type)
        {
            if (position > tokens.size())
                return type == TokenType::endOfLine;

            return tokens[position].type == type;
        }

        bool LookingAtAny(std::span<const Token> tokens, std::size_t position, std::vector<TokenType> types)
        {
            return std::ranges::any_of(types, [tokens, position](TokenType type)
                {
                    return LookingAt(tokens, position, type);
                });
        }

        std::vector<Node> CreateAlternativeNodes(std::size_t start, std::size_t end, std::span<Node> separators, std::span<std::vector<Node>> alternatives)
        {
            std::vector<Node> retval;

            for (std::size_t index = 0, max = alternatives.size(); index < max; ++index)
            {
                if (index == 0)
                {
                    const auto& rightSeparator = separators[index];
                    retval.emplace_back(NodeType::alternative, start, rightSeparator.start, alternatives[index]);
                }
                else if (index == max - 1)
                {
                    const auto& leftSeparator = separators[index - 1];
                    retval.emplace_back(NodeType::alternative, leftSeparator.end, end, alternatives[index]);
                }
                else
                {
                    const auto& rightSeparator = separators[index];
                    const auto& leftSeparator = separators[index - 1];
                    retval.emplace_back(NodeType::alternative, leftSeparator.end, rightSeparator.start, alternatives[index]);
                }
            }
            return retval;
        }

        std::vector<Node> SplitAlternatives(std::size_t start, std::size_t end, std::span<Node> alternation)
        {
            std::vector<Node> separators;
            std::vector<std::vector<Node>> alternatives{ {} };

            for (const auto& node : alternation)
            {
                if (node.type == NodeType::alternative)
                {
                    separators.push_back(node);
                    alternatives.emplace_back();
                }
                else
                    alternatives.back().push_back(node);
            }

            return CreateAlternativeNodes(start, end, separators, alternatives);
        }
    }

    ExpressionParser::Result ExpressionParser::SubParser::Parse(const ParserState& parserState) const
    {
        return this->parser(parserState, *this);
    }

    Node ExpressionParser::Parse(std::string_view expression)
    {
        tokens = ExpressionTokenizer{}.Tokenize(expression);

        ExpressionParser::SubParser parseText = { [](const ExpressionParser::ParserState& parser, const ExpressionParser::SubParser& /* subParser */) -> ExpressionParser::Result
            {
                const auto& token = parser.tokens[parser.current];
                if (MatchToken(token.type).in(TokenType::whiteSpace, TokenType::text, TokenType::endParameter, TokenType::endOptional))
                    return { 1, Node{
                                    NodeType::text,
                                    token.start,
                                    token.end,
                                    token.text,
                                } };

                if (token.type == TokenType::alternation)
                    throw AlternationNotAllowedInOptional{ parser.expression, token };

                return { 0, std::nullopt };
            } };

        ExpressionParser::SubParser parseName = { [](const ExpressionParser::ParserState& parser, const ExpressionParser::SubParser& /* subParser */) -> ExpressionParser::Result
            {
                const auto& token = parser.tokens[parser.current];
                if (MatchToken(token.type).in(TokenType::whiteSpace, TokenType::text))
                    return { 1, Node{
                                    NodeType::text,
                                    token.start,
                                    token.end,
                                    token.text,
                                } };

                if (MatchToken(token.type).in(TokenType::beginParameter, TokenType::endParameter, TokenType::beginOptional, TokenType::endOptional, TokenType::alternation))
                    throw InvalidParameterTypeNameInNode{ parser.expression, token };

                if (MatchToken(token.type).in(TokenType::startOfLine, TokenType::endOfLine))
                    return { 0, std::nullopt };

                return { 0, std::nullopt };
            } };

        auto parseParameter = ParseBetweenGenerator(NodeType::parameter, TokenType::beginParameter, TokenType::endParameter);
        parseParameter.subParsers = { parseName };

        auto parseOptional = ParseBetweenGenerator(NodeType::optional, TokenType::beginOptional, TokenType::endOptional);
        parseOptional.subParsers = { parseOptional, parseParameter, parseText };

        ExpressionParser::SubParser parseAlternativeSeparator = { [this](ParserState parser, const SubParser& subParser) -> Result
            {
                if (!LookingAt(tokens, parser.current, TokenType::alternation))
                    return { 0, std::nullopt };

                auto token = tokens[parser.current];
                return Result(1, Node{
                                     NodeType::alternative,
                                     token.start,
                                     token.end,
                                     token.text,
                                 });
            } };

        ExpressionParser::SubParser parseAlternation = { [this](const ExpressionParser::ParserState& parser, const SubParser& subParser) -> ExpressionParser::Result
            {
                auto previous = parser.current - 1;
                if (!LookingAtAny(tokens, previous, { TokenType::startOfLine, TokenType::whiteSpace, TokenType::endParameter }))
                    return { 0, std::nullopt };

                auto [consumed, ast] = ParseTokensUntil(parser.expression, subParser.subParsers, tokens, parser.current, { TokenType::whiteSpace, TokenType::endOfLine, TokenType::beginParameter });
                auto subCurrent = parser.current + consumed;
                if (std::ranges::none_of(ast, [](const auto& node)
                        {
                            return node.type == NodeType::alternative;
                        }))
                    return { 0, std::nullopt };

                auto start = tokens[parser.current].start;
                auto end = tokens[subCurrent].start;

                return { consumed, Node{
                                       NodeType::alternation,
                                       start,
                                       end,
                                       SplitAlternatives(start, end, ast),
                                   } };
            },
            {
                parseAlternativeSeparator,
                parseOptional,
                parseParameter,
                parseText,
            } };

        auto parseCucumberExpression = ParseBetweenGenerator(NodeType::expression, TokenType::startOfLine, TokenType::endOfLine);
        parseCucumberExpression.subParsers = { parseAlternation, parseOptional, parseParameter, parseText };

        auto [opt, ast] = parseCucumberExpression.Parse(ParserState{ expression, tokens, 0 });
        return *ast;
    }

    ExpressionParser::SubParser ExpressionParser::ParseBetweenGenerator(NodeType type, TokenType beginToken, TokenType endToken)
    {
        auto subParser = SubParser{ [this, type, beginToken, endToken](ParserState parser, const SubParser& subParser) -> Result
            {
                if (!LookingAt(parser.tokens, parser.current, beginToken))
                    return Result{ 0, std::nullopt };

                auto subCurrent = parser.current + 1;
                auto [consumed, ast] = ParseTokensUntil(parser.expression, subParser.subParsers, parser.tokens, subCurrent, { endToken, TokenType::endOfLine });
                subCurrent += consumed;

                // endToken not found
                if (!LookingAt(parser.tokens, subCurrent, endToken))
                    throw MissingEndToken{ parser.expression, beginToken, endToken, parser.tokens[parser.current] };

                // consumed endToken
                auto start = parser.tokens[parser.current].start;
                auto end = parser.tokens[subCurrent].end;
                consumed = subCurrent + 1 - parser.current;
                return { consumed, Node{
                                       type,
                                       start,
                                       end,
                                       ast,
                                   } };
            } };
        return subParser;
    }

    std::tuple<std::size_t, std::vector<Node>> ExpressionParser::ParseTokensUntil(std::string_view expression, std::span<const std::reference_wrapper<SubParser>> parsers, std::span<const Token> tokens, std::size_t startAt, std::vector<TokenType> endTokens)
    {
        auto current = startAt;
        auto size = tokens.size();
        std::vector<Node> ast;

        while (current < size)
        {
            if (LookingAtAny(tokens, current, endTokens))
                break;

            auto [consumed, node] = ParseToken(expression, parsers, tokens, current);
            if (consumed == 0)
                throw NoEligibleParsers{ tokens };

            current += consumed;
            ast.emplace_back(*node);
        }
        return { current - startAt, ast };
    }

    ExpressionParser::Result ExpressionParser::ParseToken(std::string_view expression, std::span<const std::reference_wrapper<SubParser>> parsers, std::span<const Token> tokens, std::size_t startAt)
    {
        for (const auto& parser : parsers)
        {
            auto [consumed, ast] = parser.get().Parse(ParserState{ expression, tokens, startAt });
            if (consumed > 0)
                return { consumed, ast };
        }

        throw NoEligibleParsers{ tokens };
    }
}
