#include "cucumber_cpp/library/tag_expression/TagExpressionParser.hpp"
#include "cucumber_cpp/library/tag_expression/Model.hpp"
#include "cucumber_cpp/library/tag_expression/TagExpressionError.hpp"
#include "cucumber_cpp/library/tag_expression/Token.hpp"
#include <cctype>
#include <cstddef>
#include <deque>
#include <format>
#include <locale>
#include <memory>
#include <stack>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::tag_expression
{
    std::unique_ptr<Expression> TagExpressionParser::Parse(std::string_view expression)
    {
        auto tokens = Tokenize(expression);

        if (tokens.empty())
            return std::make_unique<TrueExpression>();

        auto ensureExpectedTokenType = [](TokenType tokenType, TokenType expected, std::string_view lastPart, std::size_t index)
        {
            if (tokenType != expected)
                throw TagExpressionError(std::format(R"(Syntax error. Expected {} after {})", tokenTypeMap.at(expected), lastPart));
        };

        auto pushExpression = [](const Token& token, std::deque<std::unique_ptr<Expression>>& expressions)
        {
            auto requireArgCount = [&token, &expressions](std::size_t number)
            {
                if (expressions.size() < number)
                {

                    std::string expressionsStr{ "" };
                    for (const auto& expr : expressions)
                    {
                        if (!expressionsStr.empty())
                            expressionsStr += ", ";
                        expressionsStr += *expr;
                    }

                    throw TagExpressionError(std::format(R"({}: Too few operands (expressions={{{}}}))", token.keyword, expressionsStr));
                }
            };

            if (token == OR)
            {
                requireArgCount(2);

                auto term2 = std::move(expressions.back());
                expressions.pop_back();
                auto term1 = std::move(expressions.back());
                expressions.pop_back();

                expressions.push_back(std::make_unique<OrExpression>(std::move(term1), std::move(term2)));
            }
            else if (token == AND)
            {
                requireArgCount(2);

                auto term2 = std::move(expressions.back());
                expressions.pop_back();
                auto term1 = std::move(expressions.back());
                expressions.pop_back();

                expressions.push_back(std::make_unique<AndExpression>(std::move(term1), std::move(term2)));
            }
            else if (token == NOT)
            {
                requireArgCount(1);

                auto term = std::move(expressions.back());
                expressions.pop_back();

                expressions.push_back(std::make_unique<NotExpression>(std::move(term)));
            }
            else
                throw TagExpressionError(std::format("Unexpected token: {}", token.keyword));
        };

        std::stack<Token> operations;
        std::deque<std::unique_ptr<Expression>> expressions;
        std::string lastPart = "BEGIN";
        auto expectedTokenType = TokenType::operand;

        for (auto index = 0; index < tokens.size(); ++index)
        {
            const auto& part = tokens[index];
            const auto* token = SelectToken(part);

            if (token == nullptr)
            {
                ensureExpectedTokenType(TokenType::operand, expectedTokenType, lastPart, index);
                expressions.push_back(std::make_unique<LiteralExpression>(part));
                expectedTokenType = TokenType::operator_;
            }
            else if (token->IsUnary())
            {
                ensureExpectedTokenType(TokenType::operand, expectedTokenType, lastPart, index);
                operations.push(*token);
                expectedTokenType = TokenType::operand;
            }
            else if (token->IsOperation())
            {
                ensureExpectedTokenType(TokenType::operator_, expectedTokenType, lastPart, index);
                while (!operations.empty() && operations.top().IsOperation() && token->HasLowerPrecedenceThan(operations.top()))
                {
                    auto lastOperation = operations.top();
                    operations.pop();
                    pushExpression(lastOperation, expressions);
                }
                operations.push(*token);
                expectedTokenType = TokenType::operand;
            }
            else if (*token == OPEN_PARENTHESIS)
            {
                ensureExpectedTokenType(TokenType::operand, expectedTokenType, lastPart, index);
                operations.push(*token);
                expectedTokenType = TokenType::operand;
            }
            else if (*token == CLOSE_PARENTHESIS)
            {
                ensureExpectedTokenType(TokenType::operator_, expectedTokenType, lastPart, index);
                while (!operations.empty() && operations.top() != OPEN_PARENTHESIS)
                {
                    auto lastOperation = operations.top();
                    operations.pop();
                    pushExpression(lastOperation, expressions);
                }

                if (operations.empty())
                    throw TagExpressionError(std::format("Missing '(': Too few open-parens in: {}", expression));

                else if (operations.top() == OPEN_PARENTHESIS)
                {
                    operations.pop();
                    expectedTokenType = TokenType::operator_;
                }
            }

            lastPart = part;
        }

        while (!operations.empty())
        {
            auto lastOperation = operations.top();
            operations.pop();

            if (lastOperation == OPEN_PARENTHESIS)
                throw TagExpressionError(std::format("Unclosed '(': Too many open-parens in: {}", expression));

            pushExpression(lastOperation, expressions);
        }

        return std::move(expressions.back());
    }

    const Token* TagExpressionParser::SelectToken(std::string_view expression) const
    {
        if (!tokenMap.contains(expression))
            return nullptr;

        return &tokenMap.at(expression);
    }

    std::vector<std::string> TagExpressionParser::Tokenize(std::string_view expression)
    {
        std::vector<std::string> tokens{};
        auto escaped = false;
        std::string token = {};

        for (char ch : expression)
        {
            if (escaped)
            {
                if ((ch != '(' && ch != ')' && ch != '\\') && !std::isspace(ch, std::locale()))
                    throw TagExpressionError(std::format(R"(Tag expression "{}" could not be parsed because of syntax error: Illegal escape before "{}".)", expression, ch));
                token += ch;
                escaped = false;
            }
            else if (ch == '\\')
                escaped = true;
            else if (ch == '(' || ch == ')' || std::isspace(ch, std::locale()))
            {
                if (!token.empty())
                {
                    tokens.push_back(std::move(token));
                    token.clear();
                }
                if (ch != ' ')
                    tokens.push_back(std::string{ ch });
            }
            else
                token += ch;
        }

        if (!token.empty())
            tokens.push_back(std::move(token));

        return tokens;
    }
}
