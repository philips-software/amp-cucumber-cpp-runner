#ifndef FORMATTER_USAGE_FORMATTER_HPP
#define FORMATTER_USAGE_FORMATTER_HPP

#include "cucumber/messages/envelope.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"

namespace cucumber_cpp::library::formatter
{
    struct UsageFormatter
        : Formatter
    {
        using Formatter::Formatter;

        constexpr static auto name = "usage";

    private:
        void OnEnvelope(const cucumber::messages::envelope& envelope) override;
    };
}

#endif
