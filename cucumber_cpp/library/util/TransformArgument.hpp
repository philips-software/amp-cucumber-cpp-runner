#ifndef UTIL_TRANSFORM_ARGUMENT_HPP
#define UTIL_TRANSFORM_ARGUMENT_HPP

#include "cucumber/messages/step_match_argument.hpp"
#include "cucumber_cpp/library/util/Body.hpp"

namespace cucumber_cpp::library::util
{
    Argument ToArgument(const cucumber::messages::step_match_argument& argument);
}

#endif
