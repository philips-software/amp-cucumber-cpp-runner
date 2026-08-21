#ifndef UTIL_TRANSFORM_STEP_MATCH_ARGUMENTS_LIST_HPP
#define UTIL_TRANSFORM_STEP_MATCH_ARGUMENTS_LIST_HPP

#include "cucumber/messages/StepMatchArgumentsList.hpp"
#include "cucumber_cpp/library/util/Body.hpp"

namespace cucumber_cpp::library::util
{
    ExecuteArgs StepMatchArgumentsListToExecuteArgs(const cucumber::messages::StepMatchArgumentsList& args);
}

#endif
