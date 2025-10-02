#include "cucumber_cpp/library/tag_expression/Parser.hpp"
#include "cucumber_cpp/library/tag_expression/Error.hpp"
#include "cucumber_cpp/library/tag_expression/Model.hpp"
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
    namespace
    {
        void EnsureExpectedTokenType(TokenType tokenType, TokenType expected, std::string_view lastPart)
        {
            if (tokenType != expected)
                throw Error(std::format(R"(Syntax error. Expected {} after {})", TokenTypeMap().at(expected), lastPart));
        }

        void RequireArgCount(const Token& token, std::deque<std::unique_ptr<Expression>>& expressions, std::size_t number)
        {
            if (expressions.size() < number)
            {

                std::string expressionsStr{ "" };
                for (const auto& expr : expressions)
                {
                    if (!expressionsStr.empty())
                        expressionsStr += ", ";
                    expressionsStr += static_cast<std::string>(*expr);
                }

                throw Error(std::format(R"({}: Too few operands (expressions={{{}}}))", token.keyword, expressionsStr));
            }
        }

        template<class T>
        void PushBinary(const Token& token, std::deque<std::unique_ptr<Expression>>& expressions)
        {
            RequireArgCount(token, expressions, 2);

            auto term2 = std::move(expressions.back());
            expressions.pop_back();
            auto term1 = std::move(expressions.back());
            expressions.pop_back();

            expressions.push_back(std::make_unique<T>(std::move(term1), std::move(term2)));
        }

        template<class T>
        void PushUnary(const Token& token, std::deque<std::unique_ptr<Expression>>& expressions)
        {
            RequireArgCount(token, expressions, 1);

            auto term = std::move(expressions.back());
            expressions.pop_back();

            expressions.push_back(std::make_unique<T>(std::move(term)));
        }

        void PushExpression(const Token& token, std::deque<std::unique_ptr<Expression>>& expressions)
        {
            if (token == OR)
                PushBinary<OrExpression>(token, expressions);
            else if (token == AND)
                PushBinary<AndExpression>(token, expressions);
            else if (token == NOT)
                PushUnary<NotExpression>(token, expressions);
            else
                throw Error(std::format("Unexpected token: {}", token.keyword));
        }

        std::vector<std::string> Tokenize(std::string_view expression)
        {
            std::vector<std::string> tokens{};
            auto escaped = false;
            std::string token = {};

            for (char ch : expression)
            {
                if (escaped)
                {
                    if ((ch != '(' && ch != ')' && ch != '\\') && !std::isspace(ch, std::locale()))
                        throw Error(std::format(R"(Tag expression "{}" could not be parsed because of syntax error: Illegal escape before "{}".)", expression, ch));
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

        const Token* SelectToken(std::string_view expression)
        {
            if (!TokenMap().contains(expression))
                return nullptr;

            return &TokenMap().at(expression);
        }
    }

    std::unique_ptr<Expression> Parse(std::string_view expression)
    {
        auto tokens = Tokenize(expression);

        if (tokens.empty())
            return std::make_unique<TrueExpression>();

        std::stack<Token> operations;
        std::deque<std::unique_ptr<Expression>> expressions;
        std::string lastPart = "BEGIN";
        auto expectedTokenType = TokenType::operand;

        for (auto index = 0; index < tokens.size(); ++index)
        {
            const auto& part = tokens[index];

            if (const auto* token = SelectToken(part); token == nullptr)
            {
                EnsureExpectedTokenType(TokenType::operand, expectedTokenType, lastPart);
                expressions.push_back(std::make_unique<LiteralExpression>(part));
                expectedTokenType = TokenType::operator_;
            }
            else if (*token == NOT || *token == OPEN_PARENTHESIS)
            {
                EnsureExpectedTokenType(TokenType::operand, expectedTokenType, lastPart);
                operations.push(*token);
                expectedTokenType = TokenType::operand;
            }
            else if (token->IsOperation())
            {
                EnsureExpectedTokenType(TokenType::operator_, expectedTokenType, lastPart);
                while (!operations.empty() && operations.top().IsOperation() && token->HasLowerPrecedenceThan(operations.top()))
                {
                    auto lastOperation = operations.top();
                    operations.pop();
                    PushExpression(lastOperation, expressions);
                }
                operations.push(*token);
                expectedTokenType = TokenType::operand;
            }
            else if (*token == CLOSE_PARENTHESIS)
            {
                EnsureExpectedTokenType(TokenType::operator_, expectedTokenType, lastPart);
                while (!operations.empty() && operations.top() != OPEN_PARENTHESIS)
                {
                    auto lastOperation = operations.top();
                    operations.pop();
                    PushExpression(lastOperation, expressions);
                }

                if (operations.empty())
                    throw Error(std::format("Missing '(': Too few open-parens in: {}", expression));

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
                throw Error(std::format("Unclosed '(': Too many open-parens in: {}", expression));

            PushExpression(lastOperation, expressions);
        }

        return std::move(expressions.back());
    }

}
