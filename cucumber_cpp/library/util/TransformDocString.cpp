#include "cucumber_cpp/library/util/TransformDocString.hpp"
#include "cucumber/messages/PickleDocString.hpp"
#include "cucumber_cpp/library/util/DocString.hpp"
#include <optional>

namespace cucumber_cpp::library::util
{
    std::optional<DocString> TransformDocString(const std::optional<cucumber::messages::PickleDocString>& pickleDocString)
    {
        if (!pickleDocString.has_value())
            return std::nullopt;

        return DocString{ .mediaType = pickleDocString->mediaType, .content = pickleDocString->content };
    }

    std::optional<cucumber::messages::PickleDocString> TransformDocString(const std::optional<DocString>& docString)
    {
        if (!docString.has_value())
            return std::nullopt;

        return cucumber::messages::PickleDocString{ .mediaType = docString->mediaType, .content = docString->content };
    }
}
