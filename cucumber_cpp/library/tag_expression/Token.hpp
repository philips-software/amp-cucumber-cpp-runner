#ifndef TAG_EXPRESSION_TOKEN_HPP
#define TAG_EXPRESSION_TOKEN_HPP

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace cucumber_cpp::library::tag_expression
{
    enum struct Associative
    {
        left,
        right
    };

    inline const std::map<Associative, std::string_view> associativeMap{
        { Associative::left, "left" },
        { Associative::right, "right" },
    };

    enum struct TokenType
    {
        operand,
        operator_
    };

    inline const std::map<TokenType, std::string_view> tokenTypeMap{
        { TokenType::operand, "operand" },
        { TokenType::operator_, "operator" },
    };

    struct Token
    {
        constexpr Token(std::string keyword, std::int32_t precedence, std::optional<Associative> associative, TokenType type = TokenType::operand)
            : keyword{ std::move(keyword) }
            , precedence{ precedence }
            , associative{ associative }
            , type{ type }
        {}

        bool IsOperation() const;
        bool IsBinary() const;
        bool IsUnary() const;

        bool HasLowerPrecedenceThan(const Token& other) const;

        bool operator==(const Token& other) const = default;

        std::string keyword;
        std::int32_t precedence;
        std::optional<Associative> associative;
        TokenType type;
    };

    inline const Token OR{ "or", 0, Associative::left, TokenType::operator_ };
    inline const Token AND{ "and", 1, Associative::left, TokenType::operator_ };
    inline const Token NOT{ "not", 2, Associative::right, TokenType::operator_ };
    inline const Token OPEN_PARENTHESIS{ "(", -2, std::nullopt };
    inline const Token CLOSE_PARENTHESIS{ ")", -1, std::nullopt };

    inline const std::map<std::string_view, const Token&> tokenMap{
        { OR.keyword, OR },
        { AND.keyword, AND },
        { NOT.keyword, NOT },
        { OPEN_PARENTHESIS.keyword, OPEN_PARENTHESIS },
        { CLOSE_PARENTHESIS.keyword, CLOSE_PARENTHESIS }
    };
}

#endif
