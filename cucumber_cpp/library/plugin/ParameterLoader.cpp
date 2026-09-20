#include "cucumber_cpp/library/plugin/ParameterLoader.hpp"
#include "cucumber/cucumber-expressions/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/DefinitionRegistration.hpp"
#include <string>

namespace cucumber_cpp::library::plugin
{
    void ParameterLoader::Load(const support::DefinitionRegistration& registration, cucumber::cucumber_expressions::ParameterRegistry& parameterRegistry)
    {
        const auto& existingParameters = parameterRegistry.GetParameters();

        for (const auto& parameter : registration.GetRegisteredParameters())
        {
            if (!existingParameters.contains(parameter.params.name))
            {
                parameterRegistry.AddParameter(
                    cucumber::cucumber_expressions::ParameterType{
                        .name = parameter.params.name,
                        .regex = { std::string(parameter.params.regex) },
                        .isBuiltin = false,
                        .useForSnippets = parameter.params.useForSnippets,
                        .location = parameter.location,
                    });
            }
        }
    }
}
