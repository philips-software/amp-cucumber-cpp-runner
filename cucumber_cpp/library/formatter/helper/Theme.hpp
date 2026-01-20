#ifndef HELPER_THEME_HPP
#define HELPER_THEME_HPP

#include "cucumber/messages/test_step_result_status.hpp"
#include "fmt/color.h"
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::formatter::helper
{
    struct Theme
    {
        std::optional<fmt::text_style> attachment{};

        struct
        {
            std::optional<fmt::text_style> all{};
            std::optional<fmt::text_style> border{};
            std::optional<fmt::text_style> content{};
        } dataTable;

        struct
        {
            std::optional<fmt::text_style> all{};
            std::optional<fmt::text_style> content{};
            std::optional<fmt::text_style> delimiter{};
            std::optional<fmt::text_style> mediaType{};
        } docString;

        struct
        {
            std::optional<fmt::text_style> all{};
            std::optional<fmt::text_style> keyword{};
            std::optional<fmt::text_style> name{};
        } feature;

        std::optional<fmt::text_style> location{};

        struct
        {
            std::optional<fmt::text_style> all{};
            std::optional<fmt::text_style> keyword{};
            std::optional<fmt::text_style> name{};
        } rule;

        struct
        {
            std::optional<fmt::text_style> all{};
            std::optional<fmt::text_style> keyword{};
            std::optional<fmt::text_style> name{};
        } scenario;

        struct
        {
            std::optional<std::map<cucumber::messages::test_step_result_status, fmt::text_style, std::less<>>> all{};
            std::optional<std::map<cucumber::messages::test_step_result_status, std::string, std::less<>>> icon{};
            std::optional<std::map<cucumber::messages::test_step_result_status, std::string, std::less<>>> progress{};

            fmt::text_style All(cucumber::messages::test_step_result_status status, fmt::text_style defaultStyle = {}) const
            {
                if (all && all->contains(status))
                    return all->at(status);
                return defaultStyle;
            }

            std::string Icon(cucumber::messages::test_step_result_status status, std::string defaultIcon = {}) const
            {
                if (icon && icon->contains(status))
                    return icon->at(status);
                return defaultIcon;
            }

            std::string Progress(cucumber::messages::test_step_result_status status, std::string defaultProgress = {}) const
            {
                if (progress && progress->contains(status))
                    return progress->at(status);
                return defaultProgress;
            }
        } status{};

        struct
        {
            std::optional<fmt::text_style> argument{};
            std::optional<fmt::text_style> keyword{};
            std::optional<fmt::text_style> text{};
        } step;

        std::optional<fmt::text_style> tag{};

        struct
        {
            std::string bullet{};
        } symbol;
    };

    Theme CreateEmptyTheme();
    Theme CreateCucumberTheme();
    Theme CreatePlainTheme();

    Theme CreateTheme(std::string_view name);

    std::string Unstyled(const std::string& str);
}

#endif
