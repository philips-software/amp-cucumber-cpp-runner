#ifndef SUPPORT_POLYFILL_HPP
#define SUPPORT_POLYFILL_HPP

#include <cstdio>
#include <format>
#include <ostream>
#include <utility>

namespace cucumber_cpp::library::support
{
    template<typename... Args>
    void print(std::ostream& outputStream, std::format_string<Args...> fmt, Args&&... args)
    {
        outputStream << std::format(fmt, std::forward<Args>(args)...);
    }
}

#endif
