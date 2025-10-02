#ifndef TAG_EXPRESSION_ERROR_HPP
#define TAG_EXPRESSION_ERROR_HPP

#include <stdexcept>

namespace cucumber_cpp::library::tag_expression
{
    struct Error : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };
}

#endif
