#include "cucumber_cpp/library/plugin/ParameterLoader.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/DefinitionRegistration.hpp"
#include <string>

namespace cucumber_cpp::library::plugin
{
    void ParameterLoader::Load(support::DefinitionRegistration& registration, cucumber_expression::ParameterRegistry& parameterRegistry)
    {
        const auto& existingParameters = parameterRegistry.GetParameters();

        for (const auto& parameter : registration.GetRegisteredParameters())
        {
            if (!existingParameters.contains(parameter.params.name))
            {
                parameterRegistry.AddParameter(
                    cucumber_expression::ParameterType{
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
