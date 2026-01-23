#ifndef FORMATTER_USAGE_FORMATTER_HPP
#define FORMATTER_USAGE_FORMATTER_HPP

#include "cucumber/messages/envelope.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "fmt/base.h"
#include "fmt/format.h"
#include "nlohmann/json_fwd.hpp"
#include <string>

namespace cucumber_cpp::library::formatter
{
    struct UsageFormatter
        : Formatter
    {
        using Formatter::Formatter;

        constexpr static auto name = "usage";

    private:
        struct Options
        {
            explicit Options(const nlohmann::json& json);

            bool unusedOnly;
            helper::Theme theme;
        };

        void OnEnvelope(const cucumber::messages::envelope& envelope) override;

        Options options{ formatOptions.value(name, nlohmann::json::object()) };

        const std::string rowFormat = "{0}{1}{{:{1}<{{}}}}{1}{2}{1}{{:{1}<{{}}}}{1}{2}{1}{{:{1}<{{}}}}{1}{3}";
        const std::string topRow = fmt::format(fmt::runtime(rowFormat), options.theme.table.cornerTopLeft, options.theme.table.dash, options.theme.table.edgeTopT, options.theme.table.cornerTopRight);
        const std::string middleRow = fmt::format(fmt::runtime(rowFormat), options.theme.table.edgeLeftT, options.theme.table.dash, options.theme.table.cross, options.theme.table.edgeRightT);
        const std::string bottomRow = fmt::format(fmt::runtime(rowFormat), options.theme.table.cornerBottomLeft, options.theme.table.dash, options.theme.table.edgeBottomT, options.theme.table.cornerBottomRight);
    };
}

#endif
