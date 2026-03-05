#ifndef UTIL_TRANSFORM_PICKLE_TAG_HPP
#define UTIL_TRANSFORM_PICKLE_TAG_HPP

#include "cucumber/messages/pickle_tag.hpp"
#include <functional>
#include <set>
#include <span>
#include <string>

namespace cucumber_cpp::library::util
{
    std::set<std::string, std::less<>> TransformPickleTags(std::span<const cucumber::messages::pickle_tag> tags);
}

#endif
