#ifndef FORMATTER_MESSAGE_FORMATTER_HPP
#define FORMATTER_MESSAGE_FORMATTER_HPP

#include "cucumber/messages/envelope.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "nlohmann/json_fwd.hpp"
#include <optional>
#include <string>

namespace cucumber_cpp::library::formatter
{
    struct MessageFormatter
        : Formatter
    {
        using Formatter::Formatter;

        constexpr static auto name = "message";

    private:
        void OnEnvelope(const cucumber::messages::envelope& envelope) override;
    };
}

#endif
