
#include "cucumber_cpp/library/util/TransformTag.hpp"
#include "cucumber/messages/tag.hpp"
#include <functional>
#include <iterator>
#include <ranges>
#include <set>
#include <span>
#include <string>

namespace cucumber_cpp::library::util
{
    namespace
    {
        const std::string& TransformTagname(const cucumber::messages::tag& tag)
        {
            return tag.name;
        }
    }

    std::set<std::string, std::less<>> TransformTags(std::span<const cucumber::messages::tag> tags)
    {
        auto tagNames = tags | std::views::transform(TransformTagname);
        return { std::begin(tagNames), std::end(tagNames) };
    }
}
