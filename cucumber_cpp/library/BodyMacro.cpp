
#include "cucumber/messages/group.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include <gtest/gtest.h>
#include <optional>
#include <ranges>
#include <string>

namespace cucumber_cpp::library::detail
{
    namespace
    {
        std::optional<std::string> ToString(const cucumber::messages::group& group)
        {
            return group.value;
        }
    }

    cucumber_expression::ConvertFunctionArg GroupToArgumentGroup(const cucumber::messages::group& group)
    {
        if (group.children.empty())
            return { group.value };

        auto strings = group.children | std::views::transform(ToString);

        return { strings.begin(), strings.end() };
    }
}
