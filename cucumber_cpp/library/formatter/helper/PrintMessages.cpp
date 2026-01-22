#include "cucumber_cpp/library/formatter/helper/PrintMessages.hpp"
#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/hook.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber/messages/rule.hpp"
#include "cucumber/messages/scenario.hpp"
#include "cucumber/messages/step.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/formatter/helper/FormatMessages.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "cucumber_cpp/library/query/Query.hpp"
#include "fmt/format.h"
#include "fmt/ostream.h"
#include "fmt/ranges.h"
#include <algorithm>
#include <cstddef>
#include <optional>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::formatter::helper
{
    namespace
    {
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

    void PrintFeatureLine(std::ostream& stream, const cucumber::messages::feature& feature, const Theme& theme)
    {
        fmt::println(stream, "\n{}", FormatFeatureTitle(feature, theme));
    }

    void PrintRuleLine(std::ostream& stream, const cucumber::messages::rule& rule, const Theme& theme)
    {
        fmt::println(stream, "\n{}{}", std::string(gherkinIndentLength, ' '), FormatRuleTitle(rule, theme));
    }

    void PrintTags(std::ostream& stream, const cucumber::messages::pickle& pickle, std::size_t scenarioIndent, const Theme& theme)
    {
        if (pickle.tags.empty())
            return;

        fmt::println(stream, "{}{}", std::string(scenarioIndent, ' '), FormatPickleTags(pickle, theme));
    }

    void PrintScenarioLine(std::ostream& stream, const cucumber::messages::pickle& pickle, const cucumber::messages::scenario& scenario, std::size_t scenarioIndent, std::size_t maxContentLength, const Theme& theme)
    {
        PrintGherkinLine(stream,
            FormatPickleTitle(pickle, scenario, theme),
            FormatPickleLocation(pickle, scenario.location, theme),
            scenarioIndent,
            maxContentLength, theme);
    }

    void PrintScenarioAttemptLine(std::ostream& stream, const cucumber::messages::pickle& pickle, std::size_t attempt, bool retry, const cucumber::messages::scenario& scenario, std::size_t scenarioIndent, std::size_t maxContentLength, const Theme& theme)
    {
        PrintGherkinLine(stream,
            FormatPickleAttemptTitle(pickle, attempt, retry, scenario, theme),
            FormatPickleLocation(pickle, scenario.location, theme),
            scenarioIndent,
            maxContentLength, theme);
    }

    void PrintHookLine(std::ostream& stream, const cucumber::messages::test_step_finished& testStepFinished, const cucumber::messages::hook& hook, std::size_t scenarioIndent, std::size_t maxContentLength, bool isBeforeHook, bool useStatusIcon, const Theme& theme)
    {
        PrintGherkinLine(stream,
            helper::FormatHookTitle(hook, testStepFinished.test_step_result.status, isBeforeHook, useStatusIcon, theme),
            helper::FormatCodeLocation(hook.source_reference, theme),
            scenarioIndent + 2, maxContentLength, theme);
    }

    void PrintStepLine(std::ostream& stream, const cucumber::messages::test_step_finished& testStepFinished, const cucumber::messages::test_step& testStep, const cucumber::messages::pickle_step& pickleStep, const cucumber::messages::step& step, const cucumber::messages::step_definition* stepDefinition, std::size_t scenarioIndent, std::size_t maxContentLength, bool useStatusIcon, const Theme& theme)
    {
        PrintGherkinLine(stream,
            helper::FormatStepTitle(testStep, pickleStep, step, testStepFinished.test_step_result.status, useStatusIcon, theme),
            helper::FormatCodeLocation(stepDefinition, theme),
            scenarioIndent + 2, maxContentLength, theme);
    }

    void PrintStepArgument(std::ostream& stream, const cucumber::messages::pickle_step& pickleStep, std::size_t scenarioIndent, bool useStatusIcon, const helper::Theme& theme)
    {
        const auto content = FormatPickleStepArgument(pickleStep, theme);
        if (content.empty())
            return;

        PrintlnIndentedContent(stream, content, scenarioIndent + gherkinIndentLength + stepArgumentIndentLength + (useStatusIcon ? gherkinIndentLength : 0));
    }

    void PrintAmbiguousStep(std::ostream& stream, const query::Query& query, const cucumber::messages::test_step_finished& testStepFinished, const cucumber::messages::test_step& testStep, std::size_t scenarioIndent, bool useStatusIcon, const Theme& theme)
    {
        if (testStepFinished.test_step_result.status != cucumber::messages::test_step_result_status::AMBIGUOUS)
            return;

        const auto list = query.FindStepDefinitionsById(testStep);
        const auto content = FormatAmbiguousStep(list, theme);

        if (content.empty())
            return;

        PrintlnIndentedContent(stream, content, scenarioIndent + gherkinIndentLength + errorIndentLength + (useStatusIcon ? gherkinIndentLength : 0));
    }

    void PrintError(std::ostream& stream, const cucumber::messages::test_step_finished& testStepFinished, std::size_t scenarioIndent, bool useStatusIcon, const Theme& theme)
    {
        const auto content = FormatTestStepResultError(testStepFinished.test_step_result, theme);
        if (content.empty())
            return;

        PrintlnIndentedContent(stream, content, scenarioIndent + gherkinIndentLength + errorIndentLength + (useStatusIcon ? gherkinIndentLength : 0));
    }

    void PrintGherkinLine(std::ostream& stream, const std::string& title, const std::optional<std::string>& location, std::size_t indent, std::size_t maxContentLength, const Theme& theme)
    {
        const auto unstyledLength = helper::Unstyled(title).length();
        const auto padding = location.has_value() ? (maxContentLength - std::min(unstyledLength, maxContentLength)) + 1 : 0;

        const std::string paddingStr(padding, ' ');
        const auto content = fmt::format("{}{}{}", title, paddingStr, location.value_or(""));

        PrintlnIndentedContent(stream, content, indent);
    }

    void PrintAttachment(std::ostream& stream, const cucumber::messages::attachment& attachment, std::size_t scenarioIndent, bool useStatusIcon, const Theme& theme)
    {
        const auto content = FormatAttachment(attachment, theme);
        const std::string indentStr(scenarioIndent + helper::gherkinIndentLength + helper::attachmentIndentLength + (useStatusIcon ? helper::gherkinIndentLength : 0), ' ');

        fmt::println(stream, "\n{}{}\n", indentStr, content);
    }
}
