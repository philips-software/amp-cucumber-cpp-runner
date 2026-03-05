#ifndef HELPER_INDENT_STRING_HPP
#define HELPER_INDENT_STRING_HPP

#include <cstddef>
#include <string>

namespace cucumber_cpp::library::formatter::helper
{
    std::string IndentString(const std::string& str, std::size_t indentSize);
}

#endif
