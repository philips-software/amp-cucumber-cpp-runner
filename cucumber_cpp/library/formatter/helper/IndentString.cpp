#include "cucumber_cpp/library/formatter/helper/IndentString.hpp"
#include "fmt/format.h"
#include "fmt/ranges.h"
#include <cstddef>
#include <ranges>
#include <string>

namespace cucumber_cpp::library::formatter::helper
{
    std::string IndentString(const std::string& str, std::size_t indentSize)
    {
        auto lines = str |
                     std::views::split('\n') |
                     std::views::transform([indent = std::string(indentSize, ' ')](const auto& line)
                         {
                             return indent + std::string{ line.begin(), line.end() };
                         });

        return fmt::to_string(fmt::join(lines, "\n"));
    }
}
