#ifndef UTIL_TRANSFORM_TAG_HPP
#define UTIL_TRANSFORM_TAG_HPP

#include "cucumber/messages/Tag.hpp"
#include <functional>
#include <memory>
#include <set>
#include <span>
#include <string>

namespace cucumber_cpp::library::util
{
    std::set<std::string, std::less<>> TransformTags(std::span<const std::shared_ptr<cucumber::messages::Tag>> tags);
}

#endif
