#ifndef UTIL_TRIM_HPP
#define UTIL_TRIM_HPP

#include <string>

namespace cucumber_cpp::library::util
{
    [[nodiscard]] inline std::string Trim(const std::string& str, const char* whitespace = " \t\n\r")
    {
        const auto start = str.find_first_not_of(whitespace);
        if (start == std::string::npos)
            return "";

        const auto end = str.find_last_not_of(whitespace);
        return str.substr(start, end - start + 1);
    }
}

#endif
