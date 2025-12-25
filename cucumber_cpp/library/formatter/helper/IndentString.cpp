#include "cucumber_cpp/library/formatter/helper/IndentString.hpp"
#include <cstddef>
#include <format>
#include <iterator>
#include <numeric>
#include <ranges>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::formatter::helper
{
    std::string IndentString(const std::string& str, std::size_t indentSize)
    {
        using std::operator""sv;

        auto lines = str | std::views::split("\n"sv);
        auto lineCount = std::distance(lines.begin(), lines.end());

        if (lineCount == 0)
            return "";

        const auto indent = std::string(indentSize, ' ');
        std::string indented = std::format("{}{}", indent, std::string_view{ lines.front().begin(), lines.front().end() });

        for (const auto line : lines | std::views::drop(1))
            indented += std::format("\n{}{}", indent, std::string_view{ line.begin(), line.end() });

        return indented;
    }
}
