#ifndef UTIL_TO_LOWER_HPP
#define UTIL_TO_LOWER_HPP

#include <algorithm>
#include <cctype>
#include <string>

namespace cucumber_cpp::library::util
{
    [[nodiscard]] inline std::string ToLower(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c)
            {
                return std::tolower(c);
            });
        return str;
    }
}

#endif
