#ifndef CUCUMBER_EXPRESSION_AST_HPP
#define CUCUMBER_EXPRESSION_AST_HPP

#include <cstddef>
#include <string>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    enum class NodeType
    {
        text,
        optional,
        alternation,
        alternative,
        parameter,
        expression,
    };

    enum class TokenType
    {
        startOfLine,
        endOfLine,
        whiteSpace,
        beginOptional,
        endOptional,
        beginParameter,
        endParameter,
        alternation,
        text,
        invalid,
    };

    struct Node
    {
        NodeType type;
        std::size_t start;
        std::size_t end;
        std::variant<std::monostate, std::string, std::vector<Node>> children;

        std::string Text();
        std::string Text() const;

        std::vector<Node>& Children();
        const std::vector<Node>& Children() const;

        bool operator==(const Node& other) const = default;
    };

    struct Token
    {
        TokenType type;
        std::string text;
        std::size_t start;
        std::size_t end;

        bool operator==(const Token& other) const = default;

        static bool IsEscapeCharacter(unsigned char ch);
        static TokenType TypeOf(unsigned char ch);
        static bool CanEscape(unsigned char ch);
        static std::string NameOf(TokenType type);
        static std::string SymbolOf(TokenType type);
        static std::string PurposeOf(TokenType type);
    };
}

#endif
