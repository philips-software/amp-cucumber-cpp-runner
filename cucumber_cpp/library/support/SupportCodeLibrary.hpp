#ifndef SUPPORT_SUPPORT_CODE_LIBRARY_HPP
#define SUPPORT_SUPPORT_CODE_LIBRARY_HPP

#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"

namespace cucumber_cpp::library::support
{
    struct SupportCodeLibrary
    {
        HookRegistry& hookRegistry;
        StepRegistry& stepRegistry;
        cucumber_expression::ParameterRegistry& parameterRegistry;
    };
}

#endif
