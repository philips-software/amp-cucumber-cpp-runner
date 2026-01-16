#ifndef FORMATTER_JUNIT_XML_FORMATTER_HPP
#define FORMATTER_JUNIT_XML_FORMATTER_HPP

#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include <cucumber/messages/envelope.hpp>

namespace cucumber_cpp::library::formatter
{
    struct JunitXmlFormatter
        : Formatter
    {
        using Formatter::Formatter;

        constexpr static auto name = "junit";

    private:
        void OnEnvelope(const cucumber::messages::envelope& envelope) override;
    };
}

#endif
