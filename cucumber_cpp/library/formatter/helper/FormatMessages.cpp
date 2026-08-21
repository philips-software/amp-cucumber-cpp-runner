#include "cucumber_cpp/library/formatter/helper/FormatMessages.hpp"
#include "cucumber/messages/Attachment.hpp"
#include "cucumber/messages/AttachmentContentEncoding.hpp"
#include "cucumber/messages/Feature.hpp"
#include "cucumber/messages/Hook.hpp"
#include "cucumber/messages/Location.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber/messages/PickleDocString.hpp"
#include "cucumber/messages/PickleStep.hpp"
#include "cucumber/messages/PickleTable.hpp"
#include "cucumber/messages/Rule.hpp"
#include "cucumber/messages/Scenario.hpp"
#include "cucumber/messages/SourceReference.hpp"
#include "cucumber/messages/Step.hpp"
#include "cucumber/messages/StepDefinition.hpp"
#include "cucumber/messages/TestRunFinished.hpp"
#include "cucumber/messages/TestStep.hpp"
#include "cucumber/messages/TestStepResult.hpp"
#include "cucumber/messages/TestStepResultStatus.hpp"
#include "cucumber_cpp/library/formatter/helper/TextBuilder.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "cucumber_cpp/library/util/Trim.hpp"
#include "fmt/color.h"
#include "fmt/format.h"
#include "fmt/ranges.h"
#include <algorithm>
#include <cstddef>
#include <list>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::formatter::helper
{
    namespace
    {
        std::vector<std::size_t> CalculateColumnWidths(const cucumber::messages::PickleTable& pickleDataTable)
        {
            std::vector<std::size_t> columnWidths(pickleDataTable.rows.empty() ? 0 : pickleDataTable.rows.front()->cells.size(), 0);

            for (const auto& row : pickleDataTable.rows)
                for (std::size_t colIndex = 0; colIndex < row->cells.size(); ++colIndex)
                {
                    const auto cellContentLength = row->cells[colIndex]->value.length();
                    columnWidths[colIndex] = std::max(columnWidths[colIndex], cellContentLength);
                }

            return columnWidths;
        }

        const auto transformToString = [](auto subrange)
        {
            return std::string_view{ subrange.begin(), subrange.end() };
        };

        std::string GetAttemptText(std::size_t attempt, bool willBeRetried)
        {
            if (attempt > 0 || willBeRetried)
                return fmt::format("(attempt {}{})", attempt + 1, willBeRetried ? ", retried" : "");
            return "";
        }
    }

    std::string FormatPickleTitle(const cucumber::messages::Pickle& pickle, const cucumber::messages::Scenario& scenario, const Theme& theme)
    {
        return TextBuilder{}
            .Append(scenario.keyword + ":", theme.scenario.keyword)
            .Space()
            .Append(pickle.name, theme.scenario.name)
            .Build(theme.scenario.all);
    }

    std::string FormatPickleAttemptTitle(const cucumber::messages::Pickle& pickle, std::size_t attempt, bool retry, const cucumber::messages::Scenario& scenario, const Theme& theme)
    {
        auto attemptText = GetAttemptText(attempt, retry);

        TextBuilder builder{};
        builder.Append(scenario.keyword + ":", theme.scenario.keyword)
            .Space()
            .Append(pickle.name, theme.scenario.name);

        if (!attemptText.empty())
            builder.Space().Append(attemptText, theme.scenario.attempt);

        return builder.Build(theme.scenario.all);
    }

    std::string FormatPickleLocation(const cucumber::messages::Pickle& pickle, const std::shared_ptr<const cucumber::messages::Location>& location, const Theme& theme)
    {
        TextBuilder builder{};

        builder.Append("#")
            .Space()
            .Append(pickle.uri);
        if (location)
            builder.Append(":")
                .Append(std::to_string(location->line));

        return builder.Build(theme.location);
    }

    std::string FormatStepText(const cucumber::messages::TestStep& testStep, const cucumber::messages::PickleStep& pickleStep, cucumber::messages::TestStepResultStatus status, const Theme& theme)
    {
        TextBuilder builder{};
        const auto& stepMatchArgumentsLists = testStep.stepMatchArgumentsLists;

        if (stepMatchArgumentsLists && stepMatchArgumentsLists->size() == 1)
        {
            const auto& stepMatchArguments = stepMatchArgumentsLists->front()->stepMatchArguments;
            std::size_t currentIndex = 0;

            for (const auto& argument : stepMatchArguments)
            {
                const auto& group = *argument->group;

                if (group.value.has_value() && group.start.has_value())
                {
                    const auto text = pickleStep.text.substr(currentIndex, group.start.value() - currentIndex);
                    currentIndex = group.start.value() + group.value->size();
                    builder.Append(text, theme.step.text.value_or(fmt::text_style{}) | theme.status.All(status))
                        .Append(group.value.value(), theme.step.argument.value_or(fmt::text_style{}) | theme.status.All(status));
                }
            }
            if (currentIndex != pickleStep.text.size())
            {
                const auto remainingText = pickleStep.text.substr(currentIndex);
                builder.Append(remainingText, theme.step.text.value_or(fmt::text_style{}) | theme.status.All(status));
            }
        }
        else
            builder.Append(pickleStep.text, theme.step.text.value_or(fmt::text_style{}) | theme.status.All(status));

        return builder.Build();
    }

    std::string FormatCodeLocation(const cucumber::messages::SourceReference& sourceReference, const Theme& theme)
    {
        if (sourceReference.uri.has_value())
        {
            TextBuilder builder{};

            builder.Append("#")
                .Space()
                .Append(sourceReference.uri.value());

            if (sourceReference.location.has_value())
                builder.Append(":")
                    .Append(std::to_string(sourceReference.location.value()->line));
            return builder.Build(theme.location);
        }

        return "";
    }

    std::string FormatCodeLocation(const cucumber::messages::StepDefinition* stepDefinition, const Theme& theme)
    {
        if (stepDefinition != nullptr)
            return FormatCodeLocation(*stepDefinition->sourceReference, theme);

        return "";
    }

    std::string FormatFeatureTitle(const cucumber::messages::Feature& feature, const Theme& theme)
    {
        return TextBuilder{}
            .Append(feature.keyword + ":", theme.feature.keyword)
            .Space()
            .Append(feature.name, theme.feature.name)
            .Build(theme.feature.all);
    }

    std::string FormatRuleTitle(const cucumber::messages::Rule& rule, const Theme& theme)
    {
        return TextBuilder{}
            .Append(rule.keyword + ":", theme.rule.keyword)
            .Space()
            .Append(rule.name, theme.rule.name)
            .Build(theme.rule.all);
    }

    std::string FormatPickleTags(const cucumber::messages::Pickle& pickle, const Theme& theme)
    {
        if (!pickle.tags.empty())
        {
            return TextBuilder{}
                .Append(fmt::to_string(fmt::join(pickle.tags | std::views::transform([](const auto& tag)
                                                                   {
                                                                       return tag->name;
                                                                   }),
                    " ")))
                .Build(theme.tag);
        }
        return "";
    }

    std::string FormatHookTitle(const cucumber::messages::Hook& hook, cucumber::messages::TestStepResultStatus status, bool isBeforeHook, bool useStatusIcon, const Theme& theme)
    {
        TextBuilder builder{};

        if (useStatusIcon)
            builder.Append(theme.status.Icon(status, " "), theme.status.All(status)).Space();

        builder.Append((isBeforeHook ? "Before" : "After"), theme.step.keyword.value_or(fmt::text_style{}) | theme.status.All(status));

        if (hook.name.has_value())
            builder.Space().Append('(' + hook.name.value() + ')', theme.status.All(status));

        return builder.Build(theme.status.All(status));
    }

    std::string FormatStepTitle(const cucumber::messages::TestStep& testStep, const cucumber::messages::PickleStep& pickleStep, const cucumber::messages::Step& step, cucumber::messages::TestStepResultStatus status, bool useStatusIcon, const Theme& theme)
    {
        auto builder = TextBuilder{};
        if (useStatusIcon)
            builder.Append(theme.status.Icon(status, " "), theme.status.All(status)).Space();

        return builder.Append(TextBuilder{}
                                  .Append(step.keyword, theme.step.keyword)
                                  .Append(FormatStepText(testStep, pickleStep, status, theme), theme.status.All(status))
                                  .Build(theme.status.All(status)))
            .Build();
    }

    std::string FormatDocString(const cucumber::messages::PickleDocString& pickleDocString, const Theme& theme)
    {
        TextBuilder builder{};
        builder.Append(R"(""")", theme.docString.delimiter);
        if (pickleDocString.mediaType.has_value())
            builder.Append(pickleDocString.mediaType.value(), theme.docString.mediaType);
        builder.Line();

        for (const auto& line : pickleDocString.content | std::views::split('\n') | std::views::transform(transformToString))
            builder.Append(line).Line();

        builder.Append(R"(""")", theme.docString.delimiter);

        return builder.Build(theme.docString.all, true);
    }

    std::string FormatDataTable(const cucumber::messages::PickleTable& pickleDataTable, const Theme& theme)
    {
        const auto columnWidths = CalculateColumnWidths(pickleDataTable);
        TextBuilder builder{};

        for (auto rowIndex = 0; rowIndex != pickleDataTable.rows.size(); ++rowIndex)
        {
            const auto& row = pickleDataTable.rows[rowIndex];

            if (rowIndex > 0)
                builder.Line();
            builder.Append("|", theme.dataTable.border);

            for (auto colIndex = 0; colIndex != pickleDataTable.rows[rowIndex]->cells.size(); ++colIndex)
            {
                const auto& cell = row->cells[colIndex];
                builder.Append(fmt::format(" {:<{}} ", cell->value, columnWidths[colIndex]), theme.dataTable.content)
                    .Append("|", theme.dataTable.border);
            }
        }

        return builder.Build(theme.dataTable.all, true);
    }

    std::string FormatPickleStepArgument(const cucumber::messages::PickleStep& pickleStep, const Theme& theme)
    {
        if (pickleStep.argument && (*pickleStep.argument)->docString.has_value())
            return FormatDocString(*(*pickleStep.argument)->docString.value(), theme);

        if (pickleStep.argument && (*pickleStep.argument)->dataTable.has_value())
            return FormatDataTable(*(*pickleStep.argument)->dataTable.value(), theme);

        return "";
    }

    std::string FormatAmbiguousStep(const std::list<const cucumber::messages::StepDefinition*>& stepDefinitions, const Theme& theme)
    {
        TextBuilder builder{};
        builder.Append("Multiple matching step definitions found:");
        for (const auto* stepDefinition : stepDefinitions)
        {
            builder.Line().Append("  " + theme.symbol.bullet + " ");

            if (!stepDefinition->pattern->source.empty())
                builder.Append(stepDefinition->pattern->source);

            const auto location = FormatCodeLocation(stepDefinition, theme);
            if (!location.empty())
                builder.Space().Append(location);
        }
        return builder.Build({}, true);
    }

    std::string FormatTestStepResultError(const cucumber::messages::TestStepResult& testStepResult, const Theme& theme)
    {
        if (testStepResult.exception.has_value() && testStepResult.exception.value()->stackTrace.has_value())
        {
            return TextBuilder{}
                .Append(util::Trim(testStepResult.exception.value()->stackTrace.value()))
                .Build(theme.status.All(testStepResult.status), true);
        }

        if (testStepResult.exception.has_value() && testStepResult.exception.value()->message.has_value())
        {
            return TextBuilder{}
                .Append(util::Trim(testStepResult.exception.value()->type))
                .Append(": ")
                .Append(util::Trim(testStepResult.exception.value()->message.value()))
                .Build(theme.status.All(testStepResult.status), true);
        }

        if (testStepResult.message.has_value())
        {
            return TextBuilder{}
                .Append(util::Trim(testStepResult.message.value()))
                .Build(theme.status.All(testStepResult.status), true);
        }

        return "";
    }

    std::string FormatTestRunFinishedError(const cucumber::messages::TestRunFinished& testRunFinished, const Theme& theme)
    {
        if (testRunFinished.exception && (*testRunFinished.exception)->stackTrace)
        {
            return TextBuilder{}
                .Append(util::Trim((*testRunFinished.exception)->stackTrace.value()))
                .Build(theme.status.All(cucumber::messages::TestStepResultStatus::FAILED));
        }

        if (testRunFinished.exception && (*testRunFinished.exception)->message)
        {
            return TextBuilder{}
                .Append(util::Trim((*testRunFinished.exception)->message.value()))
                .Build(theme.status.All(cucumber::messages::TestStepResultStatus::FAILED));
        }

        return "";
    }

    std::string FormatBase64Attachment(const std::string& body, const std::string& mediaType, const std::optional<std::string>& filename, const Theme& theme)
    {
        TextBuilder builder{};
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

    std::string FormatTextAttachment(const std::string& body, const Theme& theme)
    {
        return TextBuilder{}.Append(body).Build(theme.attachment);
    }

    std::string FormatAttachment(const cucumber::messages::Attachment& attachment, const Theme& theme)
    {
        if (attachment.contentEncoding == cucumber::messages::AttachmentContentEncoding::BASE64)
            return FormatBase64Attachment(attachment.body, attachment.mediaType, attachment.fileName, theme);
        else
            return FormatTextAttachment(attachment.body, theme);
        return "";
    }
}
