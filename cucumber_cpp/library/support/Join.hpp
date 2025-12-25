#ifndef SUPPORT_JOIN_HPP
#define SUPPORT_JOIN_HPP

#include <initializer_list>
#include <span>
#include <string>

namespace cucumber_cpp::library::support
{
    std::string Join(std::initializer_list<const std::string> parts, const std::string& separator);
    std::string Join(std::span<const std::string> parts, const std::string& separator);
}

#endif
