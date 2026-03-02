#ifndef ENGINE_TRANSFORM_DOC_STRING_HPP
#define ENGINE_TRANSFORM_DOC_STRING_HPP

#include "cucumber/messages/pickle_doc_string.hpp"
#include "cucumber_cpp/library/util/DocString.hpp"
#include <optional>

namespace cucumber_cpp::library::util
{
    std::optional<DocString> TransformDocString(const std::optional<cucumber::messages::pickle_doc_string>& pickleDocString);
}

#endif
