
#include "cucumber_cpp/library/util/TransformPickleTag.hpp"
#include "cucumber/messages/pickle_tag.hpp"
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
        const std::string& TransformPickleTagName(const cucumber::messages::pickle_tag& tag)
        {
            return tag.name;
        }
    }

    std::set<std::string, std::less<>> TransformPickleTags(std::span<const cucumber::messages::pickle_tag> tags)
    {
        auto tagNames = tags | std::views::transform(TransformPickleTagName);
        return { std::begin(tagNames), std::end(tagNames) };
    }
}
