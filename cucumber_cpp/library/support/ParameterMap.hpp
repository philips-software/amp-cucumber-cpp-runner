#ifndef SUPPORT_PARAMETER_MAP_HPP
#define SUPPORT_PARAMETER_MAP_HPP

#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::support
{
    [[nodiscard]] std::string EscapeRegex(std::string_view text);

    // Builds a single capture group `(key1|key2|...)` with regex-escaped keys, ordered longest first.
    // Throws std::invalid_argument when keys is empty, or contains an empty or duplicate key.
    [[nodiscard]] std::string KeysToRegex(std::vector<std::string_view> keys);
}

#endif
