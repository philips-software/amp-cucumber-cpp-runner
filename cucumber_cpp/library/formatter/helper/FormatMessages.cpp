#include "cucumber_cpp/library/formatter/helper/FormatMessages.hpp"
#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/attachment_content_encoding.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/pickle_doc_string.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber/messages/pickle_table.hpp"
#include "cucumber/messages/rule.hpp"
#include "cucumber/messages/scenario.hpp"
#include "cucumber/messages/step.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/test_run_finished.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/formatter/helper/TextBuilder.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "cucumber_cpp/library/util/Trim.hpp"
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
        std::vector<std::size_t> CalculateColumnWidths(const cucumber::messages::pickle_table& pickleDataTable)
        {
            std::vector<std::size_t> columnWidths(pickleDataTable.rows.empty() ? 0 : pickleDataTable.rows.front().cells.size(), 0);

            for (const auto& row : pickleDataTable.rows)
                for (std::size_t colIndex = 0; colIndex < row.cells.size(); ++colIndex)
                {
                    const auto cellContentLength = row.cells[colIndex].value.length();
                    columnWidths[colIndex] = std::max(columnWidths[colIndex], cellContentLength);
                }

            return columnWidths;
        }

        const auto transformToString = [](auto subrange)
        {
            return std::string_view{ subrange.begin(), subrange.end() };
        };
    }

    std::string FormatPickleTitle(const cucumber::messages::pickle& pickle, const cucumber::messages::scenario& scenario, const Theme& theme)
    {
        return TextBuilder{}
            .Append(scenario.keyword + ":", theme.scenario.keyword)
            .Space()
            .Append(pickle.name, theme.scenario.name)
            .Build(theme.scenario.all);
    }

    std::string FormatPickleLocation(const cucumber::messages::pickle& pickle, const std::optional<cucumber::messages::location>& location, const Theme& theme)
    {
        TextBuilder builder{};

        builder.Append("#")
            .Space()
            .Append(pickle.uri);
        if (location.has_value())
            builder.Append(":")
                .Append(std::to_string(location.value().line));

        return builder.Build(theme.location);
    }

    std::string FormatStepText(const cucumber::messages::test_step& testStep, const cucumber::messages::pickle_step& pickleStep, const Theme& theme)
    {
        TextBuilder builder{};
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

    std::string FormatCodeLocation(const cucumber::messages::step_definition* stepDefinition, const Theme& theme)
    {
        if (stepDefinition != nullptr && stepDefinition->source_reference.uri.has_value())
        {
            TextBuilder builder{};

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

    std::string FormatFeatureTitle(const cucumber::messages::feature& feature, const Theme& theme)
    {
        return TextBuilder{}
            .Append(feature.keyword + ":", theme.feature.keyword)
            .Space()
            .Append(feature.name, theme.feature.name)
            .Build(theme.feature.all);
    }

    std::string FormatRuleTitle(const cucumber::messages::rule& rule, const Theme& theme)
    {
        return TextBuilder{}
            .Append(rule.keyword + ":", theme.rule.keyword)
            .Space()
            .Append(rule.name, theme.rule.name)
            .Build(theme.rule.all);
    }

    std::string FormatPickleTags(const cucumber::messages::pickle& pickle, const Theme& theme)
    {
        if (!pickle.tags.empty())
        {
            return TextBuilder{}
                .Append(fmt::to_string(fmt::join(pickle.tags | std::views::transform([](const auto& tag)
                                                                   {
                                                                       return tag.name;
                                                                   }),
                    " ")))
                .Build(theme.tag);
        }
        return "";
    }

    std::string FormatStepTitle(const cucumber::messages::test_step& testStep, const cucumber::messages::pickle_step& pickleStep, const cucumber::messages::step& step, cucumber::messages::test_step_result_status status, bool useStatusIcon, const Theme& theme)
    {
        auto builder = TextBuilder{};
        if (useStatusIcon)
            builder.Append(theme.status.Icon(status, " "), theme.status.All(status)).Space();

        return builder.Append(TextBuilder{}
                                  .Append(step.keyword, theme.step.keyword)
                                  .Append(FormatStepText(testStep, pickleStep, theme), theme.status.All(status))
                                  .Build(theme.status.All(status)))
            .Build();
    }

    std::string FormatDocString(const cucumber::messages::pickle_doc_string& pickleDocString, const Theme& theme)
    {
        TextBuilder builder{};
        builder.Append(R"(""")", theme.docString.delimiter);
        if (pickleDocString.media_type.has_value())
            builder.Append(pickleDocString.media_type.value(), theme.docString.mediaType);
        builder.Line();

        for (const auto& line : pickleDocString.content | std::views::split('\n') | std::views::transform(transformToString))
            builder.Append(line).Line();

        builder.Append(R"(""")", theme.docString.delimiter);

        return builder.Build(theme.docString.all, true);
    }

    std::string FormatDataTable(const cucumber::messages::pickle_table& pickleDataTable, const Theme& theme)
    {
        const auto columnWidths = CalculateColumnWidths(pickleDataTable);
        TextBuilder builder{};

        for (auto rowIndex = 0; rowIndex != pickleDataTable.rows.size(); ++rowIndex)
        {
            const auto& row = pickleDataTable.rows[rowIndex];

            if (rowIndex > 0)
                builder.Line();
            builder.Append("|", theme.dataTable.border);

            for (auto colIndex = 0; colIndex != pickleDataTable.rows[rowIndex].cells.size(); ++colIndex)
            {
                const auto& cell = row.cells[colIndex];
                builder.Append(fmt::format(" {:<{}} ", cell.value, columnWidths[colIndex]), theme.dataTable.content)
                    .Append("|", theme.dataTable.border);
            }
        }

        return builder.Build(theme.dataTable.all, true);
    }

    std::string FormatPickleStepArgument(const cucumber::messages::pickle_step& pickleStep, const Theme& theme)
    {
        if (pickleStep.argument && pickleStep.argument->doc_string.has_value())
            return FormatDocString(pickleStep.argument->doc_string.value(), theme);

        if (pickleStep.argument && pickleStep.argument->data_table.has_value())
            return FormatDataTable(pickleStep.argument->data_table.value(), theme);

        return "";
    }

    std::string FormatAmbiguousStep(const std::list<const cucumber::messages::step_definition*>& stepDefinitions, const Theme& theme)
    {
        TextBuilder builder{};
        builder.Append("Multiple matching step definitions found:");
        for (const auto* stepDefinition : stepDefinitions)
        {
            builder.Line().Append("  " + theme.symbol.bullet + " ");

            if (!stepDefinition->pattern.source.empty())
                builder.Append(stepDefinition->pattern.source);

            const auto location = FormatCodeLocation(stepDefinition, theme);
            if (!location.empty())
                builder.Space().Append(location);
        }
        return builder.Build({}, true);
    }

    std::string FormatTestStepResultError(const cucumber::messages::test_step_result& testStepResult, const Theme& theme)
    {
        if (testStepResult.exception.has_value() && testStepResult.exception.value().stack_trace.has_value())
        {
            return TextBuilder{}
                .Append(util::Trim(testStepResult.exception.value().stack_trace.value()))
                .Build(theme.status.All(testStepResult.status), true);
        }

        if (testStepResult.exception.has_value() && testStepResult.exception.value().message.has_value())
        {
            return TextBuilder{}
                .Append(util::Trim(testStepResult.exception.value().message.value()))
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

    std::string FormatTestRunFinishedError(const cucumber::messages::test_run_finished& testRunFinished, const Theme& theme)
    {
        if (testRunFinished.exception && testRunFinished.exception->stack_trace)
        {
            return TextBuilder{}
                .Append(util::Trim(testRunFinished.exception->stack_trace.value()))
                .Build(theme.status.All(cucumber::messages::test_step_result_status::FAILED));
        }

        if (testRunFinished.exception && testRunFinished.exception->message)
        {
            return TextBuilder{}
                .Append(util::Trim(testRunFinished.exception->message.value()))
                .Build(theme.status.All(cucumber::messages::test_step_result_status::FAILED));
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

    std::string FormatAttachment(const cucumber::messages::attachment& attachment, const Theme& theme)
    {
        if (attachment.content_encoding == cucumber::messages::attachment_content_encoding::BASE64)
            return FormatBase64Attachment(attachment.body, attachment.media_type, attachment.file_name, theme);
        else
            return FormatTextAttachment(attachment.body, theme);
        return "";
    }
}
