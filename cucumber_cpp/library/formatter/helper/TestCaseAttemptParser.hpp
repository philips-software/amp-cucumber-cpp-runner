#ifndef FORMATTER_TEST_CASE_ATTEMPT_PARSER_HPP
#define FORMATTER_TEST_CASE_ATTEMPT_PARSER_HPP

#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/pickle_step_argument.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::formatter::helper
{
    struct LineAndUri
    {
        std::string uri;
        std::size_t line;
    };

    struct ParsedTestCase
    {
        std::size_t attempt;
        std::string name;
        std::optional<LineAndUri> sourceLocation;
        cucumber::messages::test_step_result worstStepStepResult;
    };

    struct ParsedTestStep
    {
        std::optional<LineAndUri> actionLocation;
        std::optional<cucumber::messages::pickle_step_argument> argument;
        std::span<const cucumber::messages::attachment> attachments;
        std::string keyword;
        std::optional<std::string> name;
        const cucumber::messages::test_step_result& result;
        std::optional<LineAndUri> location;
        std::optional<std::string> text;
    };

    struct ParsedTestCaseAttempt
    {
        ParsedTestCase parsedTestCase;
        std::vector<ParsedTestStep> parsedTestSteps;
    };

    ParsedTestCaseAttempt ParseTestCaseAttempt(support::SupportCodeLibrary supportCodeLibrary, const TestCaseAttempt& testCaseAttempt);
}

#endif
