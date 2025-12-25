#include "cucumber_cpp/library/formatter/helper/TestCaseAttemptParser.hpp"
#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber_cpp/library/formatter/helper/EventDataCollector.hpp"
#include "cucumber_cpp/library/formatter/helper/GherkinDocumentParser.hpp"
#include "cucumber_cpp/library/formatter/helper/KeywordType.hpp"
#include "cucumber_cpp/library/formatter/helper/PickleParser.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include <filesystem>
#include <format>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::formatter::helper
{
    namespace
    {
        ParsedTestStep ParseStep(bool isBeforeHook,
            const GherkinStepMap& gherkinStepMap,
            std::string_view keyword,
            KeywordType keywordType,
            const std::map<std::string, const cucumber::messages::pickle_step&>& pickleStepMap,
            std::filesystem::path pickleUri,
            support::SupportCodeLibrary supportCode,
            const cucumber::messages::test_step& testStep,
            const cucumber::messages::test_step_result& testStepResult,
            std::span<const cucumber::messages::attachment> attachments)
        {
            ParsedTestStep parsedTestStep{
                .attachments = attachments,
                .keyword = std::string{ testStep.pickle_step_id ? keyword : (isBeforeHook ? "Before" : "After") },
                .result = testStepResult,
            };

            if (testStep.hook_id)
            {
                const auto& definition = supportCode.hookRegistry.GetDefinitionById(testStep.hook_id.value());
                parsedTestStep.actionLocation = {
                    .uri = definition.hook.source_reference.uri.value(),
                    .line = definition.hook.source_reference.location->line,
                };
                parsedTestStep.name = std::format(" [Hook]");
            }

            if (testStep.step_definition_ids && testStep.step_definition_ids->size() == 1)
            {
                const auto& definition = supportCode.stepRegistry.GetDefinitionById(testStep.step_definition_ids->front());
                parsedTestStep.actionLocation = {
                    .uri = definition.uri.string(),
                    .line = definition.line,
                };
            }

            if (testStep.pickle_step_id)
            {
                const auto& pickleStep = pickleStepMap.at(*testStep.pickle_step_id);

                parsedTestStep.location = {
                    .uri = pickleUri.string(),
                    .line = gherkinStepMap.at(pickleStep.ast_node_ids.front()).location.line
                };
                parsedTestStep.text = pickleStep.text;
                if (pickleStep.argument)
                    parsedTestStep.argument = *pickleStep.argument;
            }

            // if (testStepResult.status == cucumber::messages::test_step_result_status::UNDEFINED)
            //     parsedTestStep.snippet = "not supporting snippet generation yet";

            return parsedTestStep;
        }
    }

    ParsedTestCaseAttempt ParseTestCaseAttempt(support::SupportCodeLibrary& supportCodeLibrary, const TestCaseAttempt& testCaseAttempt)
    {
        const auto& testCase = testCaseAttempt.testCase;
        const auto& pickle = testCaseAttempt.pickle;
        const auto& gherkinDocument = testCaseAttempt.gherkinDocument;

        const auto gherkinStepMap = GetGherkinStepMap(gherkinDocument);
        const auto gherkinScenarioLocationMap = GetGherkinScenarioLocationMap(gherkinDocument);
        const auto pickleStepMap = GetPickleStepMap(pickle);
        const auto relativePickleUri = pickle.uri;

        const ParsedTestCase parsedTestCase{
            .attempt = testCaseAttempt.attempt,
            .name = pickle.name,
            .sourceLocation = LineAndUri{
                .uri = relativePickleUri,
                .line = gherkinScenarioLocationMap.at(pickle.ast_node_ids[0]).line,
            },
            .worstStepStepResult = testCaseAttempt.worstTestStepResult
        };

        std::vector<ParsedTestStep> parsedTestSteps{};
        parsedTestSteps.reserve(testCase.test_steps.size());

        bool isBeforeHook = true;
        std::optional<KeywordType> previousKeyWordType = KeywordType::precondition;

        for (const auto& testStep : testCase.test_steps)
        {
            const auto& testStepResult = testCaseAttempt.stepResults.at(testStep.id);
            isBeforeHook = isBeforeHook && testStep.hook_id.has_value();

            std::string_view keyword{};
            KeywordType keywordType{};
            if (testStep.pickle_step_id)
            {
                const auto& pickleStep = pickleStepMap.at(*testStep.pickle_step_id);
                keyword = GetStepKeyword(pickleStep, gherkinStepMap);
                keywordType = GetStepKeywordType(keyword, gherkinDocument.feature->language, previousKeyWordType);
            }

            parsedTestSteps.emplace_back(ParseStep(isBeforeHook,
                gherkinStepMap,
                keyword,
                keywordType,
                pickleStepMap,
                relativePickleUri,
                supportCodeLibrary,
                testStep,
                testStepResult,
                testCaseAttempt.stepAttachments.contains(testStep.id) ? testCaseAttempt.stepAttachments.at(testStep.id) : std::span<const cucumber::messages::attachment>{}));
            previousKeyWordType = keywordType;
        }

        return {
            .parsedTestCase = parsedTestCase,
            .parsedTestSteps = parsedTestSteps,
        };
    }
}
