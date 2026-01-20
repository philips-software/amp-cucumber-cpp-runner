#include "cucumber_cpp/library/formatter/PrettyFormatter.hpp"
#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/attachment_content_encoding.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/location.hpp"
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
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/formatter/helper/TextBuilder.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "fmt/format.h"
#include "fmt/ostream.h"
#include "fmt/ranges.h"
#include "nlohmann/json_fwd.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <map>
#include <optional>
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

        std::string FormatPickleTitle(const cucumber::messages::pickle& pickle, const cucumber::messages::scenario& scenario, const helper::Theme& theme = helper::CreatePlainTheme())
        {
            return helper::TextBuilder{}
                .Append(scenario.keyword + ":", theme.scenario.keyword)
                .Space()
                .Append(pickle.name, theme.scenario.name)
                .Build(theme.scenario.all);
        }

        std::string FormatPickleLocation(const cucumber::messages::pickle& pickle, const std::optional<cucumber::messages::location>& location, const helper::Theme& theme)
        {
            helper::TextBuilder builder{};

            builder.Append("#")
                .Space()
                .Append(pickle.uri);
            if (location.has_value())
                builder.Append(":")
                    .Append(std::to_string(location.value().line));

            return builder.Build(theme.location);
        }

        std::string FormatStepText(const cucumber::messages::test_step& testStep, const cucumber::messages::pickle_step& pickleStep, const helper::Theme& theme)
        {
            helper::TextBuilder builder{};
            const auto& stepMatchArgumentsLists = testStep.step_match_arguments_lists;

            if (stepMatchArgumentsLists && stepMatchArgumentsLists->size() == 1)
            {
                const auto& stepMatchArguments = stepMatchArgumentsLists->front().step_match_arguments;
                std::size_t currentIndex = 0;

                for (const auto& argument : stepMatchArguments)
                {
                    const auto& group = argument.group;

                    if (group.value.has_value() && group.start.has_value())
                    {
                        const auto text = pickleStep.text.substr(currentIndex, group.start.value() - currentIndex);
                        currentIndex = group.start.value() + group.value->size();
                        builder.Append(text, theme.step.text)
                            .Append(group.value.value(), theme.step.argument);
                    }
                }
                if (currentIndex != pickleStep.text.size())
                {
                    const auto remainingText = pickleStep.text.substr(currentIndex);
                    builder.Append(remainingText, theme.step.text);
                }
            }
            else
                builder.Append(pickleStep.text, theme.step.text);

            return builder.Build();
        }

        std::string FormatCodeLocation(const cucumber::messages::step_definition* stepDefinition, const helper::Theme& theme)
        {
            if (stepDefinition != nullptr && stepDefinition->source_reference.uri.has_value())
            {
                helper::TextBuilder builder{};

                builder.Append("#")
                    .Space()
                    .Append(stepDefinition->source_reference.uri.value());

                if (stepDefinition->source_reference.location.has_value())
                    builder.Append(":")
                        .Append(std::to_string(stepDefinition->source_reference.location.value().line));
                return builder.Build(theme.location);
            }

            return "";
        }

        std::string FormatFeatureTitle(const cucumber::messages::feature& feature, const helper::Theme& theme)
        {
            return helper::TextBuilder{}
                .Append(feature.keyword + ":", theme.feature.keyword)
                .Space()
                .Append(feature.name, theme.feature.name)
                .Build(theme.feature.all);
        }

        std::string FormatRuleTitle(const cucumber::messages::rule& rule, const helper::Theme& theme)
        {
            return helper::TextBuilder{}
                .Append(rule.keyword + ":", theme.rule.keyword)
                .Space()
                .Append(rule.name, theme.rule.name)
                .Build(theme.rule.all);
        }

        std::string FormatPickleTags(const cucumber::messages::pickle& pickle, const helper::Theme& theme)
        {
            if (!pickle.tags.empty())
            {
                return helper::TextBuilder{}
                    .Append(fmt::to_string(fmt::join(pickle.tags | std::views::transform([](const auto& tag)
                                                                       {
                                                                           return tag.name;
                                                                       }),
                        " ")))
                    .Build(theme.tag);
            }
            return "";
        }

        std::string FormatStepTitle(const cucumber::messages::test_step& testStep, const cucumber::messages::pickle_step& pickleStep, const cucumber::messages::step& step, cucumber::messages::test_step_result_status status, const PrettyFormatter::Options& options)
        {
            auto builder = helper::TextBuilder{};
            if (options.useStatusIcon)
                builder.Append(options.theme.status.Icon(status, " "), options.theme.status.All(status)).Space();

            return builder.Append(helper::TextBuilder{}
                                      .Append(step.keyword, options.theme.step.keyword)
                                      .Append(FormatStepText(testStep, pickleStep, options.theme), options.theme.status.All(status))
                                      .Build(options.theme.status.All(status)))
                .Build();
        }

        std::string FormatTestRunFinishedError(const cucumber::messages::test_run_finished& testRunFinished, const helper::Theme& theme)
        {
            if (testRunFinished.message)
            {
                return helper::TextBuilder{}
                    .Append(testRunFinished.message.value())
                    .Build(theme.status.All(cucumber::messages::test_step_result_status::FAILED));
            }
            if (testRunFinished.exception && testRunFinished.exception->stack_trace)
            {
                return helper::TextBuilder{}
                    .Append(testRunFinished.exception->stack_trace.value())
                    .Build(theme.status.All(cucumber::messages::test_step_result_status::FAILED));
            }
            if (testRunFinished.exception && testRunFinished.exception->message)
            {
                return helper::TextBuilder{}
                    .Append(testRunFinished.exception->message.value())
                    .Build(theme.status.All(cucumber::messages::test_step_result_status::FAILED));
            }
            return "";
        }

        std::string FormatBase64Attachment(const std::string& body, const std::string& mediaType, const std::optional<std::string>& filename, const helper::Theme& theme)
        {
            helper::TextBuilder builder{};
            builder.Append("Embedding").Space();

            if (filename)
                builder.Append(filename.value()).Space();

            builder
                .Append("[")
                .Append(mediaType)
                .Space()
                .Append(std::to_string(body.length() / 4 * 3))
                .Space()
                .Append("bytes]");

            return builder.Build(theme.attachment);
        }

        std::string FormatTextAttachment(const std::string& body, const helper::Theme& theme)
        {
            return helper::TextBuilder{}.Append(body).Build(theme.attachment);
        }

        std::string FormatAttachment(const cucumber::messages::attachment& attachment, const helper::Theme& theme)
        {
            if (attachment.content_encoding == cucumber::messages::attachment_content_encoding::BASE64)
                return FormatBase64Attachment(attachment.body, attachment.media_type, attachment.file_name, theme);
            else
                return FormatTextAttachment(attachment.body, theme);
            return "";
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
        const auto scenarioLength = helper::Unstyled(FormatPickleTitle(pickle, scenario, options.theme)).length();

        const auto& testCase = query.FindTestCaseBy(testCaseStarted);

        const auto hasPickleStepId = [](const cucumber::messages::test_step& testStep)
        {
            return testStep.pickle_step_id.has_value();
        };
        const auto toLength = [this](const cucumber::messages::test_step& testStep)
        {
            const auto* pickleStep = query.FindPickleStepBy(testStep);
            const auto& step = query.FindStepBy(*pickleStep);
            return helper::Unstyled(FormatStepTitle(testStep, *pickleStep, step, cucumber::messages::test_step_result_status::UNKNOWN, options)).length();
        };

        auto steplengths = testCase.test_steps | std::views::filter(hasPickleStepId) | std::views::transform(toLength);

        const auto maxStepLengthIter = std::ranges::max_element(steplengths);
        const auto maxStepLength = (maxStepLengthIter != steplengths.end()) ? *maxStepLengthIter : 0;

        maxContentLengthByTestCaseStartedId[testCaseStarted.id] = std::max(scenarioLength, options.useStatusIcon ? maxStepLength + 2 : maxStepLength);

        std::size_t scenarioIndent{ 0 };
        scenarioIndent += 2;
        if (lineage.rule)
            scenarioIndent += 2;
        scenarioIndentByTestCaseStartedId[testCaseStarted.id] = scenarioIndent;
    }

    void PrettyFormatter::HandleTestCaseStarted(const cucumber::messages::test_case_started& testCaseStarted)
    {
        const auto& pickle = query.FindPickleBy(testCaseStarted);
        const auto& location = query.FindLocationOf(pickle);
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
        }
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
            FormatStepTitle(testStep, pickleStep, step, testStepFinished.test_step_result.status, options),
            FormatCodeLocation(stepDefinition, options.theme),
            scenarioIndent + 2, maxContentLength);
    }

    void PrettyFormatter::PrintGherkinLine(const std::string& title, const std::optional<std::string>& location, std::size_t indent, std::size_t maxContentLength)
    {
        const auto unstyledLength = helper::Unstyled(title).length();
        const auto padding = location.has_value() ? (maxContentLength - std::min(unstyledLength, maxContentLength)) + 1 : 0;

        const std::string indentStr(indent, ' ');
        const std::string paddingStr(padding, ' ');

        fmt::println(outputStream, "{}{}{}{}", indentStr, title, paddingStr, location.value_or(""));
    }
}
