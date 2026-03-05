#ifndef UTIL_TRANSFORM_TAG_HPP
#define UTIL_TRANSFORM_TAG_HPP

#include "cucumber/messages/tag.hpp"
#include <functional>
#include <set>
#include <span>
#include <string>

namespace cucumber_cpp::library::util
{
    std::set<std::string, std::less<>> TransformTags(std::span<const cucumber::messages::tag> tags);
}

#endif
