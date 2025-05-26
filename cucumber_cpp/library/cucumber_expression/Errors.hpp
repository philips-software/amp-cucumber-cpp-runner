#ifndef CUCUMBER_EXPRESSION_ERRORS_HPP
#define CUCUMBER_EXPRESSION_ERRORS_HPP

#include "cucumber_cpp/library/cucumber_expression/Ast.hpp"
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string_view>

namespace cucumber_cpp::library::cucumber_expression
{
    struct Error
        : std::runtime_error
    {
        Error(std::size_t column, std::string_view expression, std::string_view pointer, std::string_view problem, std::string_view solution);
    };

    struct CantEscape
        : Error
    {
        CantEscape(std::string_view expression, std::size_t column);
    };

    struct TheEndOfLineCannotBeEscaped
        : Error
    {
        explicit TheEndOfLineCannotBeEscaped(std::string_view expression);
    };

    struct AlternationNotAllowedInOptional
        : Error
    {
        AlternationNotAllowedInOptional(std::string_view expression, const Token& token);
    };

    struct InvalidParameterTypeNameInNode
        : Error
    {
        InvalidParameterTypeNameInNode(std::string_view expression, const Token& token);
    };

    struct MissingEndToken
        : Error
    {
        MissingEndToken(std::string_view expression, TokenType beginToken, TokenType endToken, const Token& token);
    };

    struct NoEligibleParsers
        : std::runtime_error
    {
        explicit NoEligibleParsers(std::span<const Token> tokens);
    };

    struct CucumberExpressionError
        : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    struct OptionalMayNotBeEmpty
        : Error
    {
        OptionalMayNotBeEmpty(const Node& node, std::string_view expression);
    };

    struct ParameterIsNotAllowedInOptional
        : Error
    {
        ParameterIsNotAllowedInOptional(const Node& node, std::string_view expression);
    };

    struct OptionalIsNotAllowedInOptional
        : Error
    {
        OptionalIsNotAllowedInOptional(const Node& node, std::string_view expression);
    };

    struct AlternativeMayNotExclusivelyContainOptionals
        : Error
    {
        AlternativeMayNotExclusivelyContainOptionals(const Node& node, std::string_view expression);
    };

    struct AlternativeMayNotBeEmpty
        : Error
    {
        AlternativeMayNotBeEmpty(const Node& node, std::string_view expression);
    };

    struct UndefinedParameterTypeError
        : Error
    {
        UndefinedParameterTypeError(const Node& node, std::string_view expression, std::string_view undefinedParameterName);
    };

    struct InvalidTokenType
        : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    struct InvalidNodeType
        : std::logic_error
    {
        using std::logic_error::logic_error;
    };
}

#endif
