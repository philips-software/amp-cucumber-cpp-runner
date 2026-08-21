
#include "cucumber_cpp/library/util/TransformTag.hpp"
#include "cucumber/messages/Tag.hpp"
#include <functional>
#include <iterator>
#include <memory>
#include <ranges>
#include <set>
#include <span>
#include <string>

namespace cucumber_cpp::library::util
{
    std::set<std::string, std::less<>> TransformTags(std::span<const std::shared_ptr<cucumber::messages::Tag>> tags)
    {
        auto tagNames = tags | std::views::transform([](const auto& tag) -> const std::string&
                                   {
                                       return tag->name;
                                   });
        return { std::begin(tagNames), std::end(tagNames) };
    }
}
