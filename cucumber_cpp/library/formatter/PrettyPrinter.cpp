#include "cucumber_cpp/library/formatter/PrettyPrinter.hpp"
#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber/messages/pickle_tag.hpp"
#include "cucumber/messages/rule.hpp"
#include "cucumber/messages/scenario.hpp"
#include "cucumber/messages/step.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_run_finished.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber_cpp/library/formatter/GetColorFunctions.hpp"
#include "cucumber_cpp/library/support/Join.hpp"
#include "cucumber_cpp/library/support/Polyfill.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <format>
#include <functional>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::formatter
{
    namespace
    {
        std::string FormatPickleTitle(const cucumber::messages::pickle& pickle, const cucumber::messages::scenario& scenario)
        {
            return std::format("{}: {}", scenario.keyword, pickle.name);
        }

        std::string FormatStepTitle(const cucumber::messages::test_step& testStep, const cucumber::messages::pickle_step& pickleStep, const cucumber::messages::step& step)
        {
            return std::format("{}{}", step.keyword, pickleStep.text);
        }
    }

    void PrettyPrinter::OnEnvelope(const cucumber::messages::envelope& envelope)
    {
        query += envelope;

        if (envelope.test_case_started)
        {
            CalculateIndent(envelope.test_case_started.value());
            HandleTestCaseStarted(envelope.test_case_started.value());
        }

        if (envelope.test_step_finished)
        {
            HandleTestStepFinished(envelope.test_step_finished.value());
        }
    }

    void PrettyPrinter::CalculateIndent(const cucumber::messages::test_case_started& testCaseStarted)
    {
        const auto& pickle = query.FindPickleBy(testCaseStarted);
        const auto& lineage = query.FindLineageByPickle(pickle);
        const auto& scenario = *lineage.scenario;
        const auto scenarioLength = FormatPickleTitle(pickle, scenario).length();

        const auto& testCase = query.FindTestCaseBy(testCaseStarted);

        const auto hasPickleStepId = [](const cucumber::messages::test_step& testStep)
        {
            return testStep.pickle_step_id.has_value();
        };
        const auto toLength = [this](const cucumber::messages::test_step& testStep)
        {
            const auto* pickleStep = query.FindPickleStepBy(testStep);
            const auto& step = query.FindStepBy(*pickleStep);
            return FormatStepTitle(testStep, *pickleStep, step).length();
        };

        auto steplengths = testCase.test_steps | std::views::filter(hasPickleStepId) | std::views::transform(toLength);
        const auto maxStepLengthIter = std::ranges::max_element(steplengths);
        const auto maxStepLength = (maxStepLengthIter != steplengths.end()) ? *maxStepLengthIter : 0;

        maxContentLengthByTestCaseStartedId[testCaseStarted.id] = std::max(scenarioLength, maxStepLength);

        std::size_t scenarioIndent{ 0 };
        scenarioIndent += 2;
        if (lineage.rule)
            scenarioIndent += 2;
        scenarioIndentByTestCaseStartedId[testCaseStarted.id] = scenarioIndent;
    }

    void PrettyPrinter::HandleTestCaseStarted(const cucumber::messages::test_case_started& testCaseStarted)
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

    void PrettyPrinter::HandleAttachment(const cucumber::messages::attachment& attachment)
    {}

    void PrettyPrinter::HandleTestStepFinished(const cucumber::messages::test_step_finished& testStepFinished)
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

    void PrettyPrinter::HandleTestRunFinished(const cucumber::messages::test_run_finished& testRunFinished)
    {}

    void PrettyPrinter::PrintFeatureLine(const cucumber::messages::feature& feature)
    {
        if (printedFeatureUris.contains(&feature))
            return;

        support::print(outputStream, "{}: {}\n", feature.keyword, feature.name);
        printedFeatureUris.insert(&feature);
    }

    void PrettyPrinter::PrintRuleLine(const cucumber::messages::rule& rule)
    {
        if (printedRuleIds.contains(&rule))
            return;

        support::print(outputStream, "{:{}}{}: {}\n", "", 2, rule.keyword, rule.name);
        printedRuleIds.insert(&rule);
    }

    void PrettyPrinter::PrintTags(const cucumber::messages::pickle& pickle, std::size_t scenarioIndent)
    {
        if (pickle.tags.empty())
            return;

        auto tags = pickle.tags | std::views::transform([](const cucumber::messages::pickle_tag& tag)
                                      {
                                          return tag.name;
                                      });
        std::vector<std::string> tagVec{ tags.begin(), tags.end() };
        support::print(outputStream, "{:{}}{}\n", "", scenarioIndent, ColorFunctions::Tag(support::Join(tagVec, " ")));
    }

    void PrettyPrinter::PrintScenarioLine(const cucumber::messages::pickle& pickle, const cucumber::messages::scenario& scenario, std::size_t scenarioIndent, std::size_t maxContentLength)
    {
        PrintGherkinLine(std::format("{}: {}", scenario.keyword, pickle.name), nullptr, pickle.uri, scenario.location.line, scenarioIndent, maxContentLength);
    }

    void PrettyPrinter::PrintStepLine(const cucumber::messages::test_step_finished& testStepFinished, const cucumber::messages::test_step& testStep, const cucumber::messages::pickle_step& pickleStep, const cucumber::messages::step& step, const cucumber::messages::step_definition* stepDefinition, std::size_t scenarioIndent, std::size_t maxContentLength)
    {
        const auto uri = stepDefinition ? std::make_optional(*stepDefinition->source_reference.uri) : std::nullopt;
        const auto line = stepDefinition ? std::make_optional(stepDefinition->source_reference.location->line) : std::nullopt;

        PrintGherkinLine(std::format("{}{}", step.keyword, pickleStep.text), ColorFunctions::ForStatus(testStepFinished.test_step_result.status), uri, line, scenarioIndent + 2, maxContentLength);
    }

    void PrettyPrinter::PrintGherkinLine(std::string_view title, std::function<std::string(std::string_view)> formatTitle, std::optional<std::string_view> uri, std::optional<std::size_t> line, std::size_t indent, std::size_t maxContentLength)
    {
        if (title.length() > maxContentLength)
        {
            std::abort();
        }
        const auto padding = maxContentLength - title.length();

        if (uri && line)
            support::print(outputStream, "{:{}}{}{:{}} {}\n", "", indent, formatTitle ? formatTitle(title) : std::string(title), "", padding, ColorFunctions::Location(std::format("# {}:{}", *uri, *line)));
        else
            support::print(outputStream, "{:{}}{}\n", "", indent, formatTitle ? formatTitle(title) : std::string(title));
    }
}
