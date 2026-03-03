#ifndef UTIL_STEP_OR_HOOK_STARTED_HPP
#define UTIL_STEP_OR_HOOK_STARTED_HPP

#include "cucumber_cpp/library/util/TestRunHookStarted.hpp"
#include "cucumber_cpp/library/util/TestStepStarted.hpp"
#include <variant>

namespace cucumber_cpp::library::util
{
    using StepOrHookStarted = std::variant<TestStepStarted, TestRunHookStarted>;
}

#endif
