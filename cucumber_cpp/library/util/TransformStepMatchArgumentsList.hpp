#ifndef UTIL_TRANSFORM_STEP_MATCH_ARGUMENTS_LIST_HPP
#define UTIL_TRANSFORM_STEP_MATCH_ARGUMENTS_LIST_HPP

#include "cucumber/messages/step_match_arguments_list.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include <functional>
#include <string>

namespace cucumber_cpp::library::util
{
    using ConverterResolver = std::function<cucumber_expression::ErasedConverter(const std::string&)>;

    ExecuteArgs StepMatchArgumentsListToExecuteArgs(const cucumber::messages::step_match_arguments_list& args, const ConverterResolver& resolveConverter = nullptr);
}

#endif
