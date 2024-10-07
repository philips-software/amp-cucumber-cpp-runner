#ifndef ENGINE_STRINGTO_HPP
#define ENGINE_STRINGTO_HPP

#include "cucumber_cpp/library/InternalError.hpp"
#include <algorithm>
#include <cctype>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>

namespace cucumber_cpp
{
    template<class To>
    inline To StringTo(const std::string& s, std::source_location sourceLocation = std::source_location::current())
    {
        std::istringstream stream{ s };
        To to;
        stream >> to;
        if (stream.fail())
            throw InternalError{ "Cannnot convert parameter \"" + s + "\"", sourceLocation };
        return to;
    }

    template<>
    inline std::string StringTo<std::string>(const std::string& s, std::source_location /*sourceLocation*/)
    {
        return s;
    }

    namespace details
    {
        inline bool ichar_equals(char a, char b)
        {
            return std::tolower(static_cast<unsigned char>(a)) ==
                   std::tolower(static_cast<unsigned char>(b));
        }

        inline bool iequals(std::string_view lhs, std::string_view rhs)
        {
            return std::ranges::equal(lhs, rhs, ichar_equals);
        }
    }

    template<>
    inline bool StringTo<bool>(const std::string& s, std::source_location /*sourceLocation*/)
    {
        using details::iequals;

        return iequals(s, "true") || iequals(s, "1") || iequals(s, "yes") || iequals(s, "on") || iequals(s, "enabled") || iequals(s, "active");
    }
}

#endif
