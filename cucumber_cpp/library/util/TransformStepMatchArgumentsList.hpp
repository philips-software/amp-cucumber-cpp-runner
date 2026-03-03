#ifndef UTIL_TRANSFORM_STEP_MATCH_ARGUMENTS_LIST_HPP
#define UTIL_TRANSFORM_STEP_MATCH_ARGUMENTS_LIST_HPP

#include "cucumber/messages/step_match_arguments_list.hpp"
#include "cucumber_cpp/library/util/Body.hpp"

namespace cucumber_cpp::library::util
{
    ExecuteArgs StepMatchArgumentsListToExecuteArgs(const cucumber::messages::step_match_arguments_list& args);
}

#endif
