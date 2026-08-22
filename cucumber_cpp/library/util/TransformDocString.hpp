#ifndef ENGINE_TRANSFORM_DOC_STRING_HPP
#define ENGINE_TRANSFORM_DOC_STRING_HPP

#include "cucumber/messages/PickleDocString.hpp"
#include "cucumber_cpp/library/util/DocString.hpp"
#include <optional>

namespace cucumber_cpp::library::util
{
    std::optional<DocString> TransformDocString(const std::optional<cucumber::messages::PickleDocString>& pickleDocString);
    std::optional<cucumber::messages::PickleDocString> TransformDocString(const std::optional<DocString>& docString);
}

#endif
