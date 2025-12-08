#include "cucumber_cpp/library/support/Join.hpp"
#include <format>
#include <initializer_list>
#include <ranges>
#include <span>
#include <string>

namespace cucumber_cpp::library::support
{
    std::string Join(std::initializer_list<const std::string> parts, const std::string& separator)
    {
        return Join({ parts.begin(), parts.end() }, separator);
    }

    std::string Join(std::span<const std::string> parts, const std::string& separator)
    {
        if (parts.size() == 0)
            return "";

        std::string joined = *parts.begin();

        for (const auto part : parts | std::views::drop(1))
            joined += std::format("{}{}", separator, part);

        return joined;
    }
}
