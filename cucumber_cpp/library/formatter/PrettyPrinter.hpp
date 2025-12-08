#ifndef FORMATTER_PRETTY_PRINTER_HPP
#define FORMATTER_PRETTY_PRINTER_HPP

#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include <cstddef>

namespace cucumber_cpp::library::formatter
{
    struct PrettyPrinter
        : Formatter
    {
        using Formatter::Formatter;

    private:
        void OnEnvelope(const cucumber::messages::envelope& envelope) override;

        void CalculateIndent(const cucumber::messages::test_case_started& testCaseStarted);

        std::size_t scenarioIndent{ 0 };
    };
}

#endif
