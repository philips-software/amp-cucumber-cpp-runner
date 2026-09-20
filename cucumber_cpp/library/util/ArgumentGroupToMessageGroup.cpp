#include "cucumber_cpp/library/util/ArgumentGroupToMessageGroup.hpp"
#include "cucumber/cucumber-expressions/Group.hpp"
#include "cucumber/messages/Group.hpp"
#include <optional>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::util
{
    cucumber::messages::Group ArgumentGroupToMessageGroup(const cucumber::cucumber_expressions::ArgumentGroup& argumentGroup)
    {
        std::vector<cucumber::messages::Group> messageChildren;
        for (const auto& child : argumentGroup.children)
            messageChildren.push_back(ArgumentGroupToMessageGroup(child));

        cucumber::messages::Group group;
        group.children = std::move(messageChildren);
        group.start = argumentGroup.start;
        group.value = argumentGroup.value;
        return group;
    }
}
