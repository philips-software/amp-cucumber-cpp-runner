
#include "cucumber_cpp/library/util/TransformPickleTag.hpp"
#include "cucumber/messages/PickleTag.hpp"
#include <functional>
#include <iterator>
#include <memory>
#include <ranges>
#include <set>
#include <span>
#include <string>

namespace cucumber_cpp::library::util
{
    std::set<std::string, std::less<>> TransformPickleTags(std::span<const std::shared_ptr<cucumber::messages::PickleTag>> tags)
    {
        auto tagNames = tags | std::views::transform([](const auto& tag) -> const std::string&
                                   {
                                       return tag->name;
                                   });
        return { std::begin(tagNames), std::end(tagNames) };
    }
}
