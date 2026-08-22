
#include "cucumber_cpp/library/util/TransformArgument.hpp"
#include "cucumber/messages/Group.hpp"
#include "cucumber/messages/StepMatchArgument.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include <optional>
#include <ranges>
#include <string>

namespace cucumber_cpp::library::util
{
    namespace
    {
        std::optional<std::string> ToString(const cucumber::messages::Group& group)
        {
            return group.value;
        }

        cucumber_expression::ConvertFunctionArg GroupToArgumentGroup(const cucumber::messages::Group& group)
        {
            if (!group.children.has_value() || group.children->empty())
                return { group.value };

            auto strings = group.children.value() | std::views::transform([](const auto& child)
                                                        {
                                                            return ToString(*child);
                                                        });

            return { strings.begin(), strings.end() };
        }
    }

    Argument ToArgument(const cucumber::messages::StepMatchArgument& argument)
    {
        return {
            .converterName = argument.parameterTypeName.value_or(""),
            .converterArgs = GroupToArgumentGroup(*argument.group),
        };
    }
}
