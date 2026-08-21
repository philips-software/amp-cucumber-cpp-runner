#include "cucumber_cpp/library/util/ArgumentGroupToMessageGroup.hpp"
#include "cucumber/messages/Group.hpp"
#include "cucumber_cpp/library/cucumber_expression/Group.hpp"
#include <memory>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::util
{
    cucumber::messages::Group ArgumentGroupToMessageGroup(const cucumber_expression::ArgumentGroup& argumentGroup)
    {
        auto messageChildren = argumentGroup.children | std::views::transform([](const auto& child)
                                                            {
                                                                return std::make_shared<cucumber::messages::Group>(ArgumentGroupToMessageGroup(child));
                                                            });
        return {
            .children = std::optional<std::vector<std::shared_ptr<cucumber::messages::Group>>>{ std::in_place, messageChildren.begin(), messageChildren.end() },
            .start = argumentGroup.start,
            .value = argumentGroup.value,
        };
    }
}
