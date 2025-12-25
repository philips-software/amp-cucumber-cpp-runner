#ifndef FORMATTER_PRETTY_PRINTER_HPP
#define FORMATTER_PRETTY_PRINTER_HPP

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
#include "cucumber_cpp/library/Query.hpp"
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber_cpp/library/formatter/helper/GherkinDocumentParser.hpp"
#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::formatter
{
    struct PrettyPrinter
        : Formatter
    {
        using Formatter::Formatter;

    private:
        bool eventDataCollector{ false };

        void OnEnvelope(const cucumber::messages::envelope& envelope) override;

        void CalculateIndent(const cucumber::messages::test_case_started& testCaseStarted);

        void HandleTestCaseStarted(const cucumber::messages::test_case_started& testCaseStarted);
        void HandleAttachment(const cucumber::messages::attachment& attachment);
        void HandleTestStepFinished(const cucumber::messages::test_step_finished& testStepFinished);
        void HandleTestRunFinished(const cucumber::messages::test_run_finished& testRunFinished);

        void PrintFeatureLine(const cucumber::messages::feature& feature);
        void PrintRuleLine(const cucumber::messages::rule& rule);
        void PrintTags(const cucumber::messages::pickle& pickle, std::size_t scenarioIndent);
        void PrintScenarioLine(const cucumber::messages::pickle& pickle, const cucumber::messages::scenario& scenario, std::size_t scenarioIndent, std::size_t maxContentLength);
        void PrintStepLine(const cucumber::messages::test_step_finished& testStepFinished, const cucumber::messages::test_step& testStep, const cucumber::messages::pickle_step& pickleStep, const cucumber::messages::step& step, const cucumber::messages::step_definition* stepDefinition, std::size_t scenarioIndent, std::size_t maxContentLength);

        void PrintGherkinLine(std::string_view title, std::function<std::string(std::string_view)> formatTitle, std::optional<std::string_view> uri, std::optional<std::size_t> line, std::size_t indent, std::size_t maxContentLength);

        std::map<std::string, helper::GherkinScenarioMap> testCaseStartedIdToScenarioMap;

        std::map<std::string, std::size_t> maxContentLengthByTestCaseStartedId;
        std::map<std::string, std::size_t> scenarioIndentByTestCaseStartedId;

        std::set<const cucumber::messages::feature*> printedFeatureUris;
        std::set<const cucumber::messages::rule*> printedRuleIds;
    };
}

#endif
