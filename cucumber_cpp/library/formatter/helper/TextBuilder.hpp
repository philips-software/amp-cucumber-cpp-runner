#ifndef HELPER_TEXT_BUILDER_HPP
#define HELPER_TEXT_BUILDER_HPP

#include "fmt/color.h"
#include <optional>
#include <string>

namespace cucumber_cpp::library::formatter::helper
{
    struct TextBuilder
    {
        TextBuilder& Space();
        TextBuilder& Line();
        TextBuilder& Append(const std::string& text, std::optional<fmt::text_style> style = std::nullopt);
        std::string Build(std::optional<fmt::text_style> style = std::nullopt, bool styleEachLine = false) const;

    private:
        std::string text;
    };
}

#endif
