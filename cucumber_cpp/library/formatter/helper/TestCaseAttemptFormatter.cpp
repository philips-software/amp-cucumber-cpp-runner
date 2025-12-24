#include "cucumber_cpp/library/formatter/helper/TestCaseAttemptFormatter.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/formatter/GetColorFunctions.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/formatter/helper/IndentString.hpp"
#include "cucumber_cpp/library/formatter/helper/LocationHelpers.hpp"
#include "cucumber_cpp/library/formatter/helper/TestCaseAttemptParser.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include <cstddef>
#include <filesystem>
#include <format>
#include <optional>
#include <string>

namespace cucumber_cpp::library::formatter::helper
{
    namespace
    {
        std::string GetAttemptText(std::size_t attempt, bool willBeRetried)
        {
            if (attempt > 0 || willBeRetried)
                return std::format(" (attempt {}{})", attempt + 1, willBeRetried ? ", retried" : "");
            return "";
        }

        std::string GetStepMessage(const ParsedTestStep& testStep)
        {
            using enum cucumber::messages::test_step_result_status;

            switch (testStep.result.status)
            {
                case AMBIGUOUS:
                case FAILED:
                    return testStep.result.message.value_or("");

                case UNDEFINED:
                    return "Undefined. Implement with the following snippet: '<snippet>'";

                case PENDING:
                    return "Pending";

                default:
                    return {};
            }
        }

        std::string FormatStep(const ParsedTestStep& testStep, bool printAttachments)
        {
            ColorFunctions colorFunctions;
            const auto colorStatus = colorFunctions.ForStatus(testStep.result.status);

            auto identifier = std::format("{}{}", testStep.keyword, testStep.text.value_or(""));
            auto text = colorStatus(std::format("{} {}", cucumber::messages::to_string(testStep.result.status), identifier));
            if (testStep.name)
                text += colorStatus(*testStep.name);

            if (testStep.actionLocation)
                text += std::format(" # {}", colorFunctions.Location(FormatLocation(*testStep.actionLocation, std::filesystem::current_path())));

            text += '\n';

            if (testStep.argument)
                ; // indent 4 FormatStepArgument append \n

            // indent 4 print attachments append \n

            auto message = GetStepMessage(testStep);
            if (!message.empty())
                text += IndentString(colorStatus(message), 4) + "\n";

            return text;
        }
    }

    std::string FormatTestCaseAttempt(support::SupportCodeLibrary& supportCodeLibrary, const TestCaseAttempt& testCaseAttempt, bool printAttachments)
    {
        const auto parsed = ParseTestCaseAttempt(supportCodeLibrary, testCaseAttempt);

        auto text = std::format("Scenario: {}", parsed.parsedTestCase.name);
        text += GetAttemptText(parsed.parsedTestCase.attempt, testCaseAttempt.willBeRetried);
        text += std::format(" {}\n", FormatLocation(parsed.parsedTestCase.sourceLocation.value()));
        for (auto const& testStep : parsed.parsedTestSteps)
            text += FormatStep(testStep, printAttachments);
        return text + '\n';
    }
}
