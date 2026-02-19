#include "cucumber_cpp/library/cucumber_expression/Group.hpp"
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    namespace
    {
        std::optional<std::string> ToString(const ArgumentGroup& group)
        {
            return group.value;
        }
    }

    std::vector<std::optional<std::string>> ArgumentGroup::Values() const
    {
        if (children.empty())
            return { value };

        auto strings = children | std::views::transform(ToString);

        return { strings.begin(), strings.end() };
    }
}
