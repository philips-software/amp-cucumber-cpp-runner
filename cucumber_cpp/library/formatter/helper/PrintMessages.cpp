#include "cucumber_cpp/library/formatter/helper/PrintMessages.hpp"
#include "cucumber/messages/Attachment.hpp"
#include "cucumber/messages/Feature.hpp"
#include "cucumber/messages/Hook.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber/messages/PickleStep.hpp"
#include "cucumber/messages/Rule.hpp"
#include "cucumber/messages/Scenario.hpp"
#include "cucumber/messages/Step.hpp"
#include "cucumber/messages/StepDefinition.hpp"
#include "cucumber/messages/TestStep.hpp"
#include "cucumber/messages/TestStepFinished.hpp"
#include "cucumber/messages/TestStepResultStatus.hpp"
#include "cucumber/query/Query.hpp"
#include "cucumber_cpp/library/formatter/helper/FormatMessages.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "fmt/format.h"
#include "fmt/ostream.h"
#include "fmt/ranges.h"
#include <algorithm>
#include <cstddef>
#include <list>
#include <memory>
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

    void PrintFeatureLine(std::ostream& stream, const cucumber::messages::Feature& feature, const Theme& theme)
    {
        fmt::println(stream, "\n{}", FormatFeatureTitle(feature, theme));
    }

    void PrintRuleLine(std::ostream& stream, const cucumber::messages::Rule& rule, const Theme& theme)
    {
        fmt::println(stream, "\n{}{}", std::string(gherkinIndentLength, ' '), FormatRuleTitle(rule, theme));
    }

    void PrintTags(std::ostream& stream, const cucumber::messages::Pickle& pickle, std::size_t scenarioIndent, const Theme& theme)
    {
        if (pickle.tags.empty())
            return;

        fmt::println(stream, "{}{}", std::string(scenarioIndent, ' '), FormatPickleTags(pickle, theme));
    }

    void PrintScenarioLine(std::ostream& stream, const cucumber::messages::Pickle& pickle, const cucumber::messages::Scenario& scenario, std::size_t scenarioIndent, std::size_t maxContentLength, const Theme& theme)
    {
        PrintGherkinLine(stream,
            FormatPickleTitle(pickle, scenario, theme),
            FormatPickleLocation(pickle, scenario.location, theme),
            scenarioIndent,
            maxContentLength, theme);
    }

    void PrintScenarioAttemptLine(std::ostream& stream, const cucumber::messages::Pickle& pickle, std::size_t attempt, bool retry, const cucumber::messages::Scenario& scenario, std::size_t scenarioIndent, std::size_t maxContentLength, const Theme& theme) // NOSONAR: cohesive print helper
    {
        PrintGherkinLine(stream,
            FormatPickleAttemptTitle(pickle, attempt, retry, scenario, theme),
            FormatPickleLocation(pickle, scenario.location, theme),
            scenarioIndent,
            maxContentLength, theme);
    }

    void PrintHookLine(std::ostream& stream, const cucumber::messages::TestStepFinished& testStepFinished, const cucumber::messages::Hook& hook, std::size_t scenarioIndent, std::size_t maxContentLength, bool isBeforeHook, bool useStatusIcon, const Theme& theme) // NOSONAR: cohesive print helper
    {
        PrintGherkinLine(stream,
            helper::FormatHookTitle(hook, testStepFinished.testStepResult->status, isBeforeHook, useStatusIcon, theme),
            helper::FormatCodeLocation(*hook.sourceReference, theme),
            scenarioIndent + 2, maxContentLength, theme);
    }

    void PrintStepLine(std::ostream& stream, const cucumber::messages::TestStepFinished& testStepFinished, const cucumber::messages::TestStep& testStep, const cucumber::messages::PickleStep& pickleStep, const cucumber::messages::Step& step, const cucumber::messages::StepDefinition* stepDefinition, std::size_t scenarioIndent, std::size_t maxContentLength, bool useStatusIcon, const Theme& theme) // NOSONAR: cohesive print helper
    {
        PrintGherkinLine(stream,
            helper::FormatStepTitle(testStep, pickleStep, step, testStepFinished.testStepResult->status, useStatusIcon, theme),
            helper::FormatCodeLocation(stepDefinition, theme),
            scenarioIndent + 2, maxContentLength, theme);
    }

    void PrintStepArgument(std::ostream& stream, const cucumber::messages::PickleStep& pickleStep, std::size_t scenarioIndent, bool useStatusIcon, const helper::Theme& theme)
    {
        const auto content = FormatPickleStepArgument(pickleStep, theme);
        if (content.empty())
            return;

        PrintlnIndentedContent(stream, content, scenarioIndent + gherkinIndentLength + stepArgumentIndentLength + (useStatusIcon ? gherkinIndentLength : 0));
    }

    void PrintAmbiguousStep(std::ostream& stream, const cucumber::query::Query& query, const cucumber::messages::TestStepFinished& testStepFinished, const std::shared_ptr<const cucumber::messages::TestStep>& testStep, std::size_t scenarioIndent, bool useStatusIcon, const Theme& theme)
    {
        if (testStepFinished.testStepResult->status != cucumber::messages::TestStepResultStatus::AMBIGUOUS)
            return;

        const auto stepDefinitions = query.FindStepDefinitionsBy(testStep);
        std::list<const cucumber::messages::StepDefinition*> list;
        for (const auto& stepDefinition : stepDefinitions)
            list.push_back(stepDefinition.get());
        const auto content = FormatAmbiguousStep(list, theme);

        if (content.empty())
            return;

        PrintlnIndentedContent(stream, content, scenarioIndent + gherkinIndentLength + errorIndentLength + (useStatusIcon ? gherkinIndentLength : 0));
    }

    void PrintError(std::ostream& stream, const cucumber::messages::TestStepFinished& testStepFinished, std::size_t scenarioIndent, bool useStatusIcon, const Theme& theme)
    {
        const auto content = FormatTestStepResultError(*testStepFinished.testStepResult, theme);
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

    void PrintAttachment(std::ostream& stream, const cucumber::messages::Attachment& attachment, std::size_t scenarioIndent, bool useStatusIcon, const Theme& theme)
    {
        const auto content = FormatAttachment(attachment, theme);
        const std::string indentStr(scenarioIndent + helper::gherkinIndentLength + helper::attachmentIndentLength + (useStatusIcon ? helper::gherkinIndentLength : 0), ' ');

        fmt::println(stream, "\n{}{}\n", indentStr, content);
    }
}
