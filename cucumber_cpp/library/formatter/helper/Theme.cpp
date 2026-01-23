#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
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
        const std::map<cucumber::messages::test_step_result_status, fmt::text_style, std::less<>> statusColors{
            { cucumber::messages::test_step_result_status::AMBIGUOUS, fmt::fg(fmt::color::red) },
            { cucumber::messages::test_step_result_status::FAILED, fmt::fg(fmt::color::red) },
            { cucumber::messages::test_step_result_status::PASSED, fmt::fg(fmt::color::green) },
            { cucumber::messages::test_step_result_status::PENDING, fmt::fg(fmt::color::yellow) },
            { cucumber::messages::test_step_result_status::SKIPPED, fmt::fg(fmt::color::cyan) },
            { cucumber::messages::test_step_result_status::UNDEFINED, fmt::fg(fmt::color::yellow) },
            { cucumber::messages::test_step_result_status::UNKNOWN, fmt::fg(fmt::color::gray) },
        };

        const std::map<cucumber::messages::test_step_result_status, std::string, std::less<>> iconMap{
            { cucumber::messages::test_step_result_status::AMBIGUOUS, "✘" },
            { cucumber::messages::test_step_result_status::FAILED, "✘" },
            { cucumber::messages::test_step_result_status::PASSED, "✔" },
            { cucumber::messages::test_step_result_status::PENDING, "■" },
            { cucumber::messages::test_step_result_status::SKIPPED, "↷" },
            { cucumber::messages::test_step_result_status::UNDEFINED, "■" },
            { cucumber::messages::test_step_result_status::UNKNOWN, " " },
        };

        const std::map<cucumber::messages::test_step_result_status, std::string, std::less<>> progressIcons{
            { cucumber::messages::test_step_result_status::AMBIGUOUS, "A" },
            { cucumber::messages::test_step_result_status::FAILED, "F" },
            { cucumber::messages::test_step_result_status::PASSED, "." },
            { cucumber::messages::test_step_result_status::PENDING, "P" },
            { cucumber::messages::test_step_result_status::SKIPPED, "-" },
            { cucumber::messages::test_step_result_status::UNDEFINED, "U" },
            { cucumber::messages::test_step_result_status::UNKNOWN, "?" },
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
