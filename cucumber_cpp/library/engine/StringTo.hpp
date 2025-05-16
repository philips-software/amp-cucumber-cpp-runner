#ifndef ENGINE_STRINGTO_HPP
#define ENGINE_STRINGTO_HPP

#include "cucumber_cpp/library/InternalError.hpp"
#include <algorithm>
#include <any>
#include <cctype>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>

namespace cucumber_cpp::library::engine
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

    template<class To>
    To Transform(const std::variant<std::string, std::any>& variant)
    {
        if (std::holds_alternative<std::string>(variant))
            return StringTo<To>(std::get<std::string>(variant));
        else if (std::holds_alternative<std::any>(variant))
            return std::any_cast<To>(std::get<std::any>(variant));
        else
            throw InternalError{ "Cannot convert parameter" };
    }
}

#endif
