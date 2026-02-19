#include "cucumber_cpp/library/util/ArgumentGroupToMessageGroup.hpp"
#include "cucumber/messages/group.hpp"
#include "cucumber_cpp/library/cucumber_expression/Group.hpp"
#include <ranges>

namespace cucumber_cpp::library::util
{
    cucumber::messages::group ArgumentGroupToMessageGroup(const cucumber_expression::ArgumentGroup& argumentGroup)
    {
        auto messageChildren = argumentGroup.children | std::views::transform(ArgumentGroupToMessageGroup);
        return {
            .children = { messageChildren.begin(), messageChildren.end() },
            .start = argumentGroup.start,
            .value = argumentGroup.value,
        };
    }
}
