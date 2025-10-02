#ifndef TAG_EXPRESSION_TAG_EXPRESSION_PARSER_HPP
#define TAG_EXPRESSION_TAG_EXPRESSION_PARSER_HPP

#include "cucumber_cpp/library/tag_expression/Model.hpp"
#include "cucumber_cpp/library/tag_expression/Token.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::tag_expression
{
    struct TagExpressionParser
    {
        std::unique_ptr<Expression> Parse(std::string_view expression);
        const Token* SelectToken(std::string_view expression) const;

    private:
        std::vector<std::string> Tokenize(std::string_view expression);
    };
}

#endif
