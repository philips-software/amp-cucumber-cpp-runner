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
        Node(NodeType type,
            std::size_t start,
            std::size_t end,
            std::variant<std::string, std::vector<Node>> children);

        NodeType Type() const;
        std::size_t Start() const;
        std::size_t End() const;

        std::string Text();
        std::string Text() const;

        std::vector<Node>& Children();
        const std::vector<Node>& Children() const;

        const std::variant<std::string, std::vector<Node>>& GetLeafNodes() const;

        bool operator==(const Node& other) const = default;

    private:
        NodeType type;
        std::size_t start;
        std::size_t end;
        std::variant<std::string, std::vector<Node>> children;
    };

    struct Token
    {
        Token(TokenType type, std::string text, std::size_t start, std::size_t end);

        TokenType Type() const;
        std::string Text() const;
        std::size_t Start() const;
        std::size_t End() const;

        static bool IsEscapeCharacter(unsigned char ch);
        static TokenType TypeOf(unsigned char ch);
        static bool CanEscape(unsigned char ch);
        static std::string NameOf(TokenType type);
        static std::string SymbolOf(TokenType type);
        static std::string PurposeOf(TokenType type);

        bool operator==(const Token& other) const = default;

    private:
        TokenType type;
        std::string text;
        std::size_t start;
        std::size_t end;
    };
}

#endif
