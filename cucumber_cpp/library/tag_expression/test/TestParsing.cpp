#include "cucumber_cpp/library/tag_expression/Model.hpp"
#include "cucumber_cpp/library/tag_expression/TagExpressionError.hpp"
#include "yaml-cpp/node/emit.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"
#include "yaml-cpp/yaml.h"
#include "gmock/gmock.h"
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <gtest/gtest.h>
#include <locale>
#include <map>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::tag_expression
{
    ////////////////////////////////////////////////////////////////////////

    enum struct Associative
    {
        left,
        right
    };

    enum struct TokenType
    {
        operand,
        operator_
    };

    ////////////////////////////////////////////////////////////////////////

    struct Token
    {
        constexpr Token(std::string keyword, std::int32_t precedence, std::optional<Associative> associative, TokenType type = TokenType::operand)
            : keyword{ std::move(keyword) }
            , precedence{ precedence }
            , associative{ associative }
            , type{ type } {};

        bool IsOperation() const;
        bool IsBinary() const;
        bool IsUnary() const;

        bool HasLowerPrecedenceThan(const Token& other) const
        {
            return ((associative.value_or(Associative::right) == Associative::left) && precedence <= other.precedence) || //
                   ((associative.value_or(Associative::left) == Associative::right) && precedence < other.precedence);
        }

        bool operator==(const Token& other) const = default;

        std::string keyword;
        std::int32_t precedence;
        std::optional<Associative> associative;
        TokenType type;
    };

    constexpr Token OR{ "or", 0, Associative::left, TokenType::operator_ };
    constexpr Token AND{ "and", 1, Associative::left, TokenType::operator_ };
    constexpr Token NOT{ "not", 2, Associative::right, TokenType::operator_ };
    constexpr Token OPEN_PARENTHESIS{ "(", -2, std::nullopt };
    constexpr Token CLOSE_PARENTHESIS{ ")", -1, std::nullopt };

    const std::map<std::string_view, const Token&> tokenMap{
        { OR.keyword, OR },
        { AND.keyword, AND },
        { NOT.keyword, NOT },
        { OPEN_PARENTHESIS.keyword, OPEN_PARENTHESIS },
        { CLOSE_PARENTHESIS.keyword, CLOSE_PARENTHESIS }
    };

    bool Token::IsOperation() const
    {
        return type == TokenType::operator_;
    }

    bool Token::IsBinary() const
    {
        return *this == OR || *this == AND;
    }

    bool Token::IsUnary() const
    {
        return *this == NOT;
    }

    ////////////////////////////////////////////////////////////////////////

    struct TagExpressionParser
    {
        std::unique_ptr<Expression> Parse(std::string_view expression);
        const Token* SelectToken(std::string_view expression) const;

    private:
        std::vector<std::string>
        Tokenize(std::string_view expression);
    };

    ////////////////////////////////////////////////////////////////////////

    std::unique_ptr<Expression> TagExpressionParser::Parse(std::string_view expression)
    {
        auto tokens = Tokenize(expression);

        if (tokens.empty())
            return std::make_unique<TrueExpression>();

        auto ensureExpectedTokenType = [](TokenType expected, TokenType actual, std::size_t index)
        {
            if (expected != actual)
                throw 1;
            // throw TagExpressionError(std::format(R"(Syntax error. Expected {} after {})", expected, actual));
        };

        auto pushExpression = [](const Token& token, std::stack<std::unique_ptr<Expression>>& expressions)
        {
            auto requireArgCount = [&expressions](std::size_t number)
            {
                if (expressions.size() < number)
                    throw 1; // raise TagExpressionError("%s: Too few operands (expressions=%r)" % (token.keyword, expressions))
            };

            if (token == OR)
            {
                requireArgCount(2);

                auto term2 = std::move(expressions.top());
                expressions.pop();
                auto term1 = std::move(expressions.top());
                expressions.pop();

                expressions.push(std::make_unique<OrExpression>(std::move(term1), std::move(term2)));
            }
            else if (token == AND)
            {
                requireArgCount(2);

                auto term2 = std::move(expressions.top());
                expressions.pop();
                auto term1 = std::move(expressions.top());
                expressions.pop();

                expressions.push(std::make_unique<AndExpression>(std::move(term1), std::move(term2)));
            }
            else if (token == NOT)
            {
                requireArgCount(1);

                auto term = std::move(expressions.top());
                expressions.pop();

                expressions.push(std::make_unique<NotExpression>(std::move(term)));
            }
            else
                throw 1; // TagExpressionError("Unexpected token: %r" % token)
        };

        std::stack<Token> operations;
        std::stack<std::unique_ptr<Expression>> expressions;
        std::string lastPart = "BEGIN";
        auto expectedTokenType = TokenType::operand;

        for (auto index = 0; index < tokens.size(); ++index)
        {
            const auto& part = tokens[index];
            const auto* token = SelectToken(part);

            if (token == nullptr)
            {
                ensureExpectedTokenType(TokenType::operand, expectedTokenType, index);
                expressions.push(std::make_unique<LiteralExpression>(part));
                expectedTokenType = TokenType::operator_;
            }
            else if (token->IsUnary())
            {
                ensureExpectedTokenType(TokenType::operand, expectedTokenType, index);
                operations.push(*token);
                expectedTokenType = TokenType::operand;
            }
            else if (token->IsOperation())
            {
                ensureExpectedTokenType(TokenType::operator_, expectedTokenType, index);
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
                ensureExpectedTokenType(TokenType::operand, expectedTokenType, index);
                operations.push(*token);
                expectedTokenType = TokenType::operand;
            }
            else if (*token == CLOSE_PARENTHESIS)
            {
                ensureExpectedTokenType(TokenType::operator_, expectedTokenType, index);
                while (!operations.empty() && operations.top() != OPEN_PARENTHESIS)
                {
                    auto lastOperation = operations.top();
                    operations.pop();
                    pushExpression(lastOperation, expressions);
                }

                if (operations.empty())
                    throw 1; // TagExpressionError("Missing '(': Too few open-parens in: %s" % text)
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
                throw 1; // TagExpressionError(Unclosed '(': Too many open-parens in: %s)

            pushExpression(lastOperation, expressions);
        }

        return std::move(expressions.top());
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

    ////////////////////////////////////////////////////////////////////////
    struct TestParsingFixture : testing::Test
    {
    };

    struct TestParsing : TestParsingFixture

    {
        TestParsing(std::string_view expression, std::string_view formatted)
            : expression{ expression }
            , formatted{ formatted }
        {}

        void TestBody() override
        {
            const auto tagExpression = TagExpressionParser{}.Parse(expression);

            ASSERT_THAT(tagExpression, testing::NotNull());
            const auto actualText = static_cast<std::string>(*tagExpression);
            EXPECT_THAT(actualText, testing::StrEq(formatted));
        }

        std::string expression;
        std::string formatted;
    };

    namespace
    {
        std::vector<std::pair<std::string, YAML::Node>> GetTestData(const std::filesystem::path& path)
        {
            std::vector<std::pair<std::string, YAML::Node>> testdata;

            if (std::filesystem::is_regular_file(path) && (path.extension() == ".yml" || path.extension() == ".yaml"))
                testdata.emplace_back(path.string(), YAML::LoadFile(path.string()));
            else
                for (const auto& file : std::filesystem::directory_iterator(path))
                    if (file.is_regular_file() && (file.path().extension() == ".yml" || file.path().extension() == ".yaml"))
                        testdata.emplace_back(file.path().string(), YAML::LoadFile(file.path().string()));

            return testdata;
        }

        std::vector<testing::TestInfo*> RegisterMyTests()
        {
            std::vector<testing::TestInfo*> tests;
            std::filesystem::path testdataPath = "testdata/tag-expression/parsing.yml";

            std::size_t lineNumber = 2;

            for (const auto& [file, testdata] : GetTestData(testdataPath))
            {
                for (const auto& node : testdata)
                {
                    auto factory = [node = node]() -> TestParsingFixture*
                    {
                        return new TestParsing(node["expression"].as<std::string>(), node["formatted"].as<std::string>());
                    };

                    auto* testInfo = testing::RegisterTest("TestParsing", std::format("Test_{}_{}", tests.size(), lineNumber).c_str(), nullptr, nullptr, testdataPath.c_str(), lineNumber, factory);

                    tests.push_back(testInfo);

                    lineNumber += 2;
                }
            }

            return tests;
        }
    }

    auto TestParsing = RegisterMyTests();
}
