#include "cucumber_cpp/library/tag_expression/Token.hpp"

namespace cucumber_cpp::library::tag_expression
{
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

    bool Token::HasLowerPrecedenceThan(const Token& other) const
    {
        return ((associative.value_or(Associative::right) == Associative::left) && precedence <= other.precedence) || //
               ((associative.value_or(Associative::left) == Associative::right) && precedence < other.precedence);
    }
}
