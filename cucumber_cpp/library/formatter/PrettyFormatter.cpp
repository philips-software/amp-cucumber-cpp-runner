#include "cucumber_cpp/library/formatter/PrettyFormatter.hpp"
#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber/messages/rule.hpp"
#include "cucumber/messages/scenario.hpp"
#include "cucumber/messages/step.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_run_finished.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/formatter/helper/FormatMessages.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "fmt/format.h"
#include "fmt/ostream.h"
#include "fmt/ranges.h"
#include "nlohmann/json_fwd.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <map>
#include <optional>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::formatter
{
    namespace
    {
        constexpr auto gherkinIndentLength = 2;
        constexpr auto stepArgumentIndentLength = 2;
        constexpr auto attachmentIndentLength = 4;
        constexpr auto errorIndentLength = 4;

        const auto transformToString = [](auto subrange)
        {
            return std::string_view{ subrange.begin(), subrange.end() };
        };

        void PrintlnIndentedContent(std::ostream& os, std::string_view content, std::size_t indent)
        {
            const std::string indentStr(indent, ' ');
            fmt::println(os, "{}{}", indentStr, fmt::join(content | std::views::split('\n') | std::views::transform(transformToString), "\n" + indentStr));
        }

    }

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
        {
            HandleAttachment(envelope.attachment.value());
        }

        if (envelope.test_step_finished)
        {
            HandleTestStepFinished(envelope.test_step_finished.value());
        }

        if (envelope.test_run_finished)
        {
            HandleTestRunFinished(envelope.test_run_finished.value());
        }
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
            scenarioIndent += gherkinIndentLength;
            if (options.includeRuleLine && lineage.rule)
                scenarioIndent += gherkinIndentLength;
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

        PrintFeatureLine(*feature);
        if (rule)
            PrintRuleLine(*rule);
        outputStream << "\n";
        PrintTags(pickle, scenarioIndent);
        PrintScenarioLine(pickle, *scenario, scenarioIndent, maxContentLength);
    }

    void PrettyFormatter::HandleAttachment(const cucumber::messages::attachment& attachment)
    {
        if (!options.includeAttachments)
            return;

        const auto scenarioIndent = scenarioIndentByTestCaseStartedId.at(attachment.test_case_started_id.value());
        const auto content = FormatAttachment(attachment, options.theme);
        const std::string indentStr(scenarioIndent + gherkinIndentLength + attachmentIndentLength + (options.useStatusIcon ? gherkinIndentLength : 0), ' ');

        fmt::println(outputStream, "\n{}{}\n", indentStr, content);
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

            PrintStepLine(testStepFinished, testStep, *pickleStep, step, stepDefinition, scenarioIndent, maxContentLength);
            PrintStepArgument(*pickleStep, scenarioIndent, options.theme);
            PrintAmbiguousStep(testStepFinished, testStep, scenarioIndent);
        }
        PrintError(testStepFinished, scenarioIndent);
    }

    void PrettyFormatter::HandleTestRunFinished(const cucumber::messages::test_run_finished& testRunFinished)
    {
        const auto content = FormatTestRunFinishedError(testRunFinished, options.theme);

        if (!content.empty())
            fmt::println(outputStream, "{}", content);
    }

    void PrettyFormatter::PrintFeatureLine(const cucumber::messages::feature& feature)
    {
        if (options.includeFeatureLine == false || printedFeatureUris.contains(&feature))
            return;

        fmt::println(outputStream, "\n{}", FormatFeatureTitle(feature, options.theme));
        printedFeatureUris.insert(&feature);
    }

    void PrettyFormatter::PrintRuleLine(const cucumber::messages::rule& rule)
    {
        if (options.includeRuleLine == false || printedRuleIds.contains(&rule))
            return;

        fmt::println(outputStream, "\n{}{}", std::string(gherkinIndentLength, ' '), FormatRuleTitle(rule, options.theme));

        printedRuleIds.insert(&rule);
    }

    void PrettyFormatter::PrintTags(const cucumber::messages::pickle& pickle, std::size_t scenarioIndent)
    {
        if (pickle.tags.empty())
            return;

        fmt::println(outputStream, "{}{}", std::string(scenarioIndent, ' '), FormatPickleTags(pickle, options.theme));
    }

    void PrettyFormatter::PrintScenarioLine(const cucumber::messages::pickle& pickle, const cucumber::messages::scenario& scenario, std::size_t scenarioIndent, std::size_t maxContentLength)
    {
        PrintGherkinLine(
            FormatPickleTitle(pickle, scenario, options.theme),
            FormatPickleLocation(pickle, scenario.location, options.theme),
            scenarioIndent,
            maxContentLength);
    }

    void PrettyFormatter::PrintStepLine(const cucumber::messages::test_step_finished& testStepFinished, const cucumber::messages::test_step& testStep, const cucumber::messages::pickle_step& pickleStep, const cucumber::messages::step& step, const cucumber::messages::step_definition* stepDefinition, std::size_t scenarioIndent, std::size_t maxContentLength)
    {
        PrintGherkinLine(
            helper::FormatStepTitle(testStep, pickleStep, step, testStepFinished.test_step_result.status, options.useStatusIcon, options.theme),
            helper::FormatCodeLocation(stepDefinition, options.theme),
            scenarioIndent + 2, maxContentLength);
    }

    void PrettyFormatter::PrintStepArgument(const cucumber::messages::pickle_step& pickleStep, std::size_t scenarioIndent, const helper::Theme& theme)
    {
        const auto content = FormatPickleStepArgument(pickleStep, options.theme);
        if (content.empty())
            return;

        PrintlnIndentedContent(outputStream, content, scenarioIndent + gherkinIndentLength + stepArgumentIndentLength + (options.useStatusIcon ? gherkinIndentLength : 0));
    }

    void PrettyFormatter::PrintAmbiguousStep(const cucumber::messages::test_step_finished& testStepFinished, const cucumber::messages::test_step& testStep, std::size_t scenarioIndent)
    {
        if (testStepFinished.test_step_result.status != cucumber::messages::test_step_result_status::AMBIGUOUS)
            return;

        const auto list = query.FindStepDefinitionsById(testStep);
        const auto content = FormatAmbiguousStep(list, options.theme);

        if (content.empty())
            return;

        PrintlnIndentedContent(outputStream, content, scenarioIndent + gherkinIndentLength + errorIndentLength + (options.useStatusIcon ? gherkinIndentLength : 0));
    }

    void PrettyFormatter::PrintError(const cucumber::messages::test_step_finished& testStepFinished, std::size_t scenarioIndent)
    {
        const auto content = FormatTestStepResultError(testStepFinished.test_step_result, options.theme);
        if (content.empty())
            return;

        PrintlnIndentedContent(outputStream, content, scenarioIndent + gherkinIndentLength + errorIndentLength + (options.useStatusIcon ? gherkinIndentLength : 0));
    }

    void PrettyFormatter::PrintGherkinLine(const std::string& title, const std::optional<std::string>& location, std::size_t indent, std::size_t maxContentLength)
    {
        const auto unstyledLength = helper::Unstyled(title).length();
        const auto padding = location.has_value() ? (maxContentLength - std::min(unstyledLength, maxContentLength)) + 1 : 0;

        const std::string paddingStr(padding, ' ');
        const auto content = fmt::format("{}{}{}", title, paddingStr, location.value_or(""));

        PrintlnIndentedContent(outputStream, content, indent);
    }
}
