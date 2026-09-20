#ifndef PLUGIN_PARAMETER_LOADER_HPP
#define PLUGIN_PARAMETER_LOADER_HPP

#include "cucumber/cucumber-expressions/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/DefinitionRegistration.hpp"

namespace cucumber_cpp::library::plugin
{
    struct ParameterLoader
    {
        static void Load(const support::DefinitionRegistration& registration, cucumber::cucumber_expressions::ParameterRegistry& parameterRegistry);
    };
}

#endif
