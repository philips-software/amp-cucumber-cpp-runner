#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "cucumber/messages/TestStepResultStatus.hpp"
#include "fmt/color.h"
#include <functional>
#include <map>
#include <optional>
#include <regex>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::formatter::helper
{
    namespace
    {
        const std::map<cucumber::messages::TestStepResultStatus, fmt::text_style, std::less<>> statusColors{
            { cucumber::messages::TestStepResultStatus::AMBIGUOUS, fmt::fg(fmt::color::red) },
            { cucumber::messages::TestStepResultStatus::FAILED, fmt::fg(fmt::color::red) },
            { cucumber::messages::TestStepResultStatus::PASSED, fmt::fg(fmt::color::green) },
            { cucumber::messages::TestStepResultStatus::PENDING, fmt::fg(fmt::color::yellow) },
            { cucumber::messages::TestStepResultStatus::SKIPPED, fmt::fg(fmt::color::cyan) },
            { cucumber::messages::TestStepResultStatus::UNDEFINED, fmt::fg(fmt::color::yellow) },
            { cucumber::messages::TestStepResultStatus::UNKNOWN, fmt::fg(fmt::color::gray) },
        };

        const std::map<cucumber::messages::TestStepResultStatus, std::string, std::less<>> iconMap{
            { cucumber::messages::TestStepResultStatus::AMBIGUOUS, "✘" },
            { cucumber::messages::TestStepResultStatus::FAILED, "✘" },
            { cucumber::messages::TestStepResultStatus::PASSED, "✔" },
            { cucumber::messages::TestStepResultStatus::PENDING, "■" },
            { cucumber::messages::TestStepResultStatus::SKIPPED, "↷" },
            { cucumber::messages::TestStepResultStatus::UNDEFINED, "■" },
            { cucumber::messages::TestStepResultStatus::UNKNOWN, " " },
        };

        const std::map<cucumber::messages::TestStepResultStatus, std::string, std::less<>> progressIcons{
            { cucumber::messages::TestStepResultStatus::AMBIGUOUS, "A" },
            { cucumber::messages::TestStepResultStatus::FAILED, "F" },
            { cucumber::messages::TestStepResultStatus::PASSED, "." },
            { cucumber::messages::TestStepResultStatus::PENDING, "P" },
            { cucumber::messages::TestStepResultStatus::SKIPPED, "-" },
            { cucumber::messages::TestStepResultStatus::UNDEFINED, "U" },
            { cucumber::messages::TestStepResultStatus::UNKNOWN, "?" },
        };

        std::optional<fmt::text_style> GetColorStyle(std::optional<fmt::text_style> def)
        {
            return def;
        }

        const std::regex ansiEscape{ "\033\\[[^m]+m" };
    }

    Theme CreateEmptyTheme()
    {
        return {};
    }

    Theme CreateCucumberTheme()
    {
        static const Theme theme{
            .attachment = fmt::fg(fmt::color::cyan),
            .feature = {
                .keyword = fmt::emphasis::bold,
            },
            .location = fmt::fg(fmt::terminal_color::bright_black),
            .rule = {
                .keyword = fmt::emphasis::bold,
            },
            .scenario = {
                .keyword = fmt::emphasis::bold,
                .attempt = fmt::emphasis::italic,
            },
            .status = {
                .all = statusColors,
                .icon = iconMap,
                .progress = progressIcons,
            },
            .step = {
                .argument = fmt::emphasis::bold,
                .keyword = fmt::emphasis::bold,
            },
            .symbol = { .bullet = "•" },
            .table{
                .cornerTopLeft{ "┌" },
                .cornerTopRight{ "┐" },
                .cornerBottomLeft{ "└" },
                .cornerBottomRight{ "┘" },
                .edgeTopT{ "┬" },
                .edgeBottomT{ "┴" },
                .edgeLeftT{ "├" },
                .edgeRightT{ "┤" },
                .dash{ "─" },
                .vertical{ "│" },
                .cross{ "┼" },
            },
        };

        return theme;
    }

    Theme CreatePlainTheme()
    {
        static const Theme theme{
            .status = {
                .icon = iconMap,
                .progress = progressIcons,
            },
            .symbol = { .bullet = "-" },
        };

        return theme;
    }

    Theme CreateTheme(std::string_view name)
    {
        if (name == "cucumber")
            return CreateCucumberTheme();
        else if (name == "plain")
            return CreatePlainTheme();
        else if (name == "none")
            return CreateEmptyTheme();
        else
            return CreateEmptyTheme();
    }

    std::string Unstyled(const std::string& str)
    {
        return std::regex_replace(str, ansiEscape, "");
    }
}
