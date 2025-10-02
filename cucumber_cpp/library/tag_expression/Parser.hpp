#ifndef TAG_EXPRESSION_PARSER_HPP
#define TAG_EXPRESSION_PARSER_HPP

#include "cucumber_cpp/library/tag_expression/Model.hpp"
#include <memory>
#include <string_view>

namespace cucumber_cpp::library::tag_expression
{
    std::unique_ptr<Expression> Parse(std::string_view expression);
}

#endif
