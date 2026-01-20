
#include "cucumber_cpp/library/formatter/helper/TextBuilder.hpp"
#include "fmt/color.h"
#include "fmt/format.h"
#include "fmt/ranges.h"
#include <optional>
#include <ranges>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::formatter::helper
{
    namespace
    {
        std::string ApplyStyle(std::string_view text, std::optional<fmt::text_style> style)
        {
            if (!style)
                return std::string{ text };

            return fmt::format(*style, "{}", text);
        }
    }

    TextBuilder& TextBuilder::Space()
    {
        text += ' ';
        return *this;
    }

    TextBuilder& TextBuilder::Line()
    {
        text += '\n';
        return *this;
    }

    TextBuilder& TextBuilder::Append(std::string_view text, std::optional<fmt::text_style> style)
    {
        this->text += ApplyStyle(text, style);
        return *this;
    }

    std::string TextBuilder::Build(std::optional<fmt::text_style> style, bool styleEachLine) const
    {
        if (styleEachLine)
        {
            auto styled = text |
                          std::views::split('\n') |
                          std::views::transform([&style](const auto& line)
                              {
                                  return ApplyStyle(std::string{ line.begin(), line.end() }, style);
                              });

            return fmt::to_string(fmt::join(styled, "\n"));
        }

        return ApplyStyle(text, style);
    }
}
