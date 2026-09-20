#ifndef UTIL_ARGUMENT_GROUP_TO_MESSAGE_GROUP_HPP
#define UTIL_ARGUMENT_GROUP_TO_MESSAGE_GROUP_HPP

#include "cucumber/cucumber-expressions/Group.hpp"
#include "cucumber/messages/Group.hpp"

namespace cucumber_cpp::library::util
{
    cucumber::messages::Group ArgumentGroupToMessageGroup(const cucumber::cucumber_expressions::ArgumentGroup& argumentGroup);
}

#endif
