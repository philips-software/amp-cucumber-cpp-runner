#include "cucumber_cpp/library/formatter/PrettyFormatter.hpp"
#include "cucumber/messages/Attachment.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/TestCaseStarted.hpp"
#include "cucumber/messages/TestRunFinished.hpp"
#include "cucumber/messages/TestStep.hpp"
#include "cucumber/messages/TestStepFinished.hpp"
#include "cucumber/messages/TestStepResultStatus.hpp"
#include "cucumber_cpp/library/formatter/helper/FormatMessages.hpp"
#include "cucumber_cpp/library/formatter/helper/PrintMessages.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "fmt/ostream.h"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>

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

    void PrettyFormatter::OnEnvelope(const cucumber::messages::Envelope& envelope)
    {
        if (envelope.testCaseStarted)
        {
            CalculateIndent(envelope.testCaseStarted.value());
            HandleTestCaseStarted(envelope.testCaseStarted.value());
        }

        if (envelope.attachment)
            HandleAttachment(envelope.attachment.value());

        if (envelope.testStepFinished)
            HandleTestStepFinished(envelope.testStepFinished.value());

        if (envelope.testRunFinished)
            HandleTestRunFinished(envelope.testRunFinished.value());
    }

    void PrettyFormatter::CalculateIndent(const cucumber::messages::TestCaseStarted& testCaseStarted)
    {
        const auto* pickle = query.FindPickleBy(testCaseStarted);
        const auto lineageAndPickle = query.FindLineageBy(*pickle).value();
        const auto& lineage = *lineageAndPickle.lineage;
        const auto& scenario = *lineage.scenario;
        const auto scenarioLength = helper::Unstyled(helper::FormatPickleTitle(*pickle, scenario, options.theme)).length();

        const auto* testCase = query.FindTestCaseBy(testCaseStarted);

        const auto hasPickleStepId = [](const cucumber::messages::TestStep& testStep)
        {
            return testStep.pickleStepId.has_value();
        };
        const auto toLength = [this](const cucumber::messages::TestStep& testStep)
        {
            const auto* pickleStep = query.FindPickleStepBy(testStep);
            const auto* step = query.FindStepBy(*pickleStep);
            return helper::Unstyled(helper::FormatStepTitle(testStep, *pickleStep, *step, cucumber::messages::TestStepResultStatus::UNKNOWN, options.useStatusIcon, options.theme)).length();
        };

        auto steplengths = testCase->testSteps | std::views::filter(hasPickleStepId) | std::views::transform(toLength);

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

    void PrettyFormatter::HandleTestCaseStarted(const cucumber::messages::TestCaseStarted& testCaseStarted)
    {
        const auto* pickle = query.FindPickleBy(testCaseStarted);
        const auto lineageAndPickle = query.FindLineageBy(*pickle).value();
        const auto& lineage = *lineageAndPickle.lineage;
        const auto* scenario = lineage.scenario;
        const auto* rule = lineage.rule;
        const auto* feature = lineage.feature;

        const auto scenarioIndent = scenarioIndentByTestCaseStartedId.at(testCaseStarted.id);
        const auto maxContentLength = maxContentLengthByTestCaseStartedId.at(testCaseStarted.id);

        if (options.includeFeatureLine && rule && !printedFeatureUris.contains(feature))
            helper::PrintFeatureLine(outputStream, *feature, options.theme);

        if (options.includeRuleLine && rule && !printedRuleIds.contains(rule))
            helper::PrintRuleLine(outputStream, *rule, options.theme);

        outputStream << "\n";

        helper::PrintTags(outputStream, *pickle, scenarioIndent, options.theme);
        helper::PrintScenarioLine(outputStream, *pickle, *scenario, scenarioIndent, maxContentLength, options.theme);

        printedFeatureUris.insert(feature);
        printedRuleIds.insert(rule);
    }

    void PrettyFormatter::HandleAttachment(const cucumber::messages::Attachment& attachment)
    {
        if (!options.includeAttachments)
            return;

        if (attachment.testCaseStartedId.has_value())
            helper::PrintAttachment(outputStream, attachment, scenarioIndentByTestCaseStartedId.at(attachment.testCaseStartedId.value()), options.useStatusIcon, options.theme);
    }

    void PrettyFormatter::HandleTestStepFinished(const cucumber::messages::TestStepFinished& testStepFinished)
    {
        const auto scenarioIndent = scenarioIndentByTestCaseStartedId.at(testStepFinished.testCaseStartedId);
        const auto maxContentLength = maxContentLengthByTestCaseStartedId.at(testStepFinished.testCaseStartedId);

        const auto* testStep = query.FindTestStepBy(testStepFinished);

        if (const auto* pickleStep = query.FindPickleStepBy(*testStep); pickleStep != nullptr)
        {
            const auto* step = query.FindStepBy(*pickleStep);
            const auto* stepDefinition = query.FindUnambiguousStepDefinitionBy(*testStep);

            helper::PrintStepLine(outputStream, testStepFinished, *testStep, *pickleStep, *step, stepDefinition, scenarioIndent, maxContentLength, options.useStatusIcon, options.theme);
            helper::PrintStepArgument(outputStream, *pickleStep, scenarioIndent, options.useStatusIcon, options.theme);
            helper::PrintAmbiguousStep(outputStream, query, testStepFinished, *testStep, scenarioIndent, options.useStatusIcon, options.theme);
        }

        helper::PrintError(outputStream, testStepFinished, scenarioIndent, options.useStatusIcon, options.theme);
    }

    void PrettyFormatter::HandleTestRunFinished(const cucumber::messages::TestRunFinished& testRunFinished)
    {
        const auto content = FormatTestRunFinishedError(testRunFinished, options.theme);

        if (!content.empty())
            fmt::println(outputStream, "{}", content);
    }
}
