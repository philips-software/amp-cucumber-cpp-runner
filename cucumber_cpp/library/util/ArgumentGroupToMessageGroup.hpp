#ifndef UTIL_ARGUMENT_GROUP_TO_MESSAGE_GROUP_HPP
#define UTIL_ARGUMENT_GROUP_TO_MESSAGE_GROUP_HPP

#include "cucumber/messages/Group.hpp"
#include "cucumber_cpp/library/cucumber_expression/Group.hpp"

namespace cucumber_cpp::library::util
{
    cucumber::messages::Group ArgumentGroupToMessageGroup(const cucumber_expression::ArgumentGroup& argumentGroup);
}

#endif
