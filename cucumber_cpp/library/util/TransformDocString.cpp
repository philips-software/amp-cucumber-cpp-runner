#include "cucumber_cpp/library/util/TransformDocString.hpp"
#include "cucumber/messages/pickle_doc_string.hpp"
#include "cucumber_cpp/library/util/DocString.hpp"
#include <optional>

namespace cucumber_cpp::library::util
{
    std::optional<DocString> TransformDocString(const std::optional<cucumber::messages::pickle_doc_string>& pickleDocString)
    {
        if (!pickleDocString.has_value())
            return std::nullopt;

        return DocString{ .mediaType = pickleDocString->media_type, .content = pickleDocString->content };
    }
}
