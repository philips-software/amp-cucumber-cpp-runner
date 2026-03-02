#ifndef UTIL_DOC_STRING_HPP
#define UTIL_DOC_STRING_HPP

#include <optional>
#include <string>

namespace cucumber_cpp::library::util
{
    struct DocString
    {
        const std::optional<std::string>& mediaType;
        const std::string& content;
    };
}

#endif
