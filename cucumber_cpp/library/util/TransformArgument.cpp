
#include "cucumber_cpp/library/util/TransformArgument.hpp"
#include "cucumber/messages/group.hpp"
#include "cucumber/messages/step_match_argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include <optional>
#include <ranges>
#include <string>

namespace cucumber_cpp::library::util
{
    namespace
    {
        std::optional<std::string> ToString(const cucumber::messages::group& group)
        {
            return group.value;
        }

        cucumber_expression::ConvertFunctionArg GroupToArgumentGroup(const cucumber::messages::group& group)
        {
            if (!group.children.has_value() || group.children->empty())
                return { group.value };

            auto strings = group.children.value() | std::views::transform(ToString);

            return { strings.begin(), strings.end() };
        }
    }

    Argument ToArgument(const cucumber::messages::step_match_argument& argument)
    {
        return {
            .converterName = argument.parameter_type_name.value_or(""),
            .converterArgs = GroupToArgumentGroup(argument.group),
        };
    }
}
