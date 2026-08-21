#ifndef UTIL_TRANSFORM_PICKLE_TAG_HPP
#define UTIL_TRANSFORM_PICKLE_TAG_HPP

#include "cucumber/messages/PickleTag.hpp"
#include <functional>
#include <memory>
#include <set>
#include <span>
#include <string>

namespace cucumber_cpp::library::util
{
    std::set<std::string, std::less<>> TransformPickleTags(std::span<const std::shared_ptr<cucumber::messages::PickleTag>> tags);
}

#endif
