#include "cucumber_cpp/library/formatter/PrettyFormatter.hpp"
#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_run_finished.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/formatter/helper/FormatMessages.hpp"
#include "cucumber_cpp/library/formatter/helper/PrintMessages.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "fmt/ostream.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::formatter
{
    PrettyFormatter::Options::Options(const nlohmann::json& formatOptions)
        : includeAttachments{ formatOptions.value("include_attachments", true) }
        , includeFeatureLine{ formatOptions.value("include_feature_line", true) }
        , includeRuleLine{ formatOptions.value("include_rule_line", true) }
        , useStatusIcon{ formatOptions.value("use_status_icon", true) }
        , theme{ helper::CreateTheme(formatOptions.value("theme", std::string_view{ "cucumber" })) }
    {
    }

    void PrettyFormatter::OnEnvelope(const cucumber::messages::envelope& envelope)
    {
        if (envelope.test_case_started)
        {
            CalculateIndent(envelope.test_case_started.value());
            HandleTestCaseStarted(envelope.test_case_started.value());
        }

        if (envelope.attachment)
            HandleAttachment(envelope.attachment.value());

        if (envelope.test_step_finished)
            HandleTestStepFinished(envelope.test_step_finished.value());

        if (envelope.test_run_finished)
            HandleTestRunFinished(envelope.test_run_finished.value());
    }

    void PrettyFormatter::CalculateIndent(const cucumber::messages::test_case_started& testCaseStarted)
    {
        const auto& pickle = query.FindPickleBy(testCaseStarted);
        const auto& lineage = query.FindLineageByPickle(pickle);
        const auto& scenario = *lineage.scenario;
        const auto scenarioLength = helper::Unstyled(helper::FormatPickleTitle(pickle, scenario, options.theme)).length();

        const auto& testCase = query.FindTestCaseBy(testCaseStarted);

        const auto hasPickleStepId = [](const cucumber::messages::test_step& testStep)
        {
            return testStep.pickle_step_id.has_value();
        };
        const auto toLength = [this](const cucumber::messages::test_step& testStep)
        {
            const auto* pickleStep = query.FindPickleStepBy(testStep);
            const auto& step = query.FindStepBy(*pickleStep);
            return helper::Unstyled(helper::FormatStepTitle(testStep, *pickleStep, step, cucumber::messages::test_step_result_status::UNKNOWN, options.useStatusIcon, options.theme)).length();
        };

        auto steplengths = testCase.test_steps | std::views::filter(hasPickleStepId) | std::views::transform(toLength);

        const auto maxStepLengthIter = std::ranges::max_element(steplengths);
        const auto maxStepLength = (maxStepLengthIter != steplengths.end()) ? *maxStepLengthIter : 0;

        maxContentLengthByTestCaseStartedId[testCaseStarted.id] = std::max(scenarioLength, options.useStatusIcon ? maxStepLength + 2 : maxStepLength);

        std::size_t scenarioIndent{ 0 };
        if (options.includeFeatureLine)
        {
            scenarioIndent += helper::gherkinIndentLength;
            if (options.includeRuleLine && lineage.rule)
                scenarioIndent += helper::gherkinIndentLength;
        }

        scenarioIndentByTestCaseStartedId[testCaseStarted.id] = scenarioIndent;
    }

    void PrettyFormatter::HandleTestCaseStarted(const cucumber::messages::test_case_started& testCaseStarted)
    {
        const auto& pickle = query.FindPickleBy(testCaseStarted);
        const auto& lineage = query.FindLineageByPickle(pickle);
        const auto& scenario = lineage.scenario;
        const auto& rule = lineage.rule;
        const auto& feature = lineage.feature;

        const auto scenarioIndent = scenarioIndentByTestCaseStartedId.at(testCaseStarted.id);
        const auto maxContentLength = maxContentLengthByTestCaseStartedId.at(testCaseStarted.id);

        if (options.includeFeatureLine && rule && !printedFeatureUris.contains(feature.get()))
            helper::PrintFeatureLine(outputStream, *feature, options.theme);

        if (options.includeRuleLine && rule && !printedRuleIds.contains(rule.get()))
            helper::PrintRuleLine(outputStream, *rule, options.theme);

        outputStream << "\n";

        helper::PrintTags(outputStream, pickle, scenarioIndent, options.theme);
        helper::PrintScenarioLine(outputStream, pickle, *scenario, scenarioIndent, maxContentLength, options.theme);

        printedFeatureUris.insert(feature.get());
        printedRuleIds.insert(rule.get());
    }

    void PrettyFormatter::HandleAttachment(const cucumber::messages::attachment& attachment)
    {
        if (!options.includeAttachments)
            return;

        if (attachment.test_case_started_id.has_value())
            helper::PrintAttachment(outputStream, attachment, scenarioIndentByTestCaseStartedId.at(attachment.test_case_started_id.value()), options.useStatusIcon, options.theme);
    }

    void PrettyFormatter::HandleTestStepFinished(const cucumber::messages::test_step_finished& testStepFinished)
    {
        const auto scenarioIndent = scenarioIndentByTestCaseStartedId.at(testStepFinished.test_case_started_id);
        const auto maxContentLength = maxContentLengthByTestCaseStartedId.at(testStepFinished.test_case_started_id);

        const auto& testStep = query.FindTestStepBy(testStepFinished);
        const auto* pickleStep = query.FindPickleStepBy(testStep);

        if (pickleStep != nullptr)
        {
            const auto& step = query.FindStepBy(*pickleStep);
            const auto* stepDefinition = (testStep.step_definition_ids && !testStep.step_definition_ids->empty()) ? &query.FindStepDefinitionById(testStep.step_definition_ids->front()) : nullptr;

            helper::PrintStepLine(outputStream, testStepFinished, testStep, *pickleStep, step, stepDefinition, scenarioIndent, maxContentLength, options.useStatusIcon, options.theme);
            helper::PrintStepArgument(outputStream, *pickleStep, scenarioIndent, options.useStatusIcon, options.theme);
            helper::PrintAmbiguousStep(outputStream, query, testStepFinished, testStep, scenarioIndent, options.useStatusIcon, options.theme);
        }

        helper::PrintError(outputStream, testStepFinished, scenarioIndent, options.useStatusIcon, options.theme);
    }

    void PrettyFormatter::HandleTestRunFinished(const cucumber::messages::test_run_finished& testRunFinished)
    {
        const auto content = FormatTestRunFinishedError(testRunFinished, options.theme);

        if (!content.empty())
            fmt::println(outputStream, "{}", content);
    }
}
