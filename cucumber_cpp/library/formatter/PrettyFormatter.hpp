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
#include "cucumber_cpp/library/formatter/Formatter.hpp"
#include "cucumber_cpp/library/formatter/helper/GherkinDocumentParser.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "cucumber_cpp/library/query/Query.hpp"
#include <cstddef>
#include <functional>
#include <map>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <set>
#include <string>
#include <string_view>

namespace cucumber_cpp::library::formatter
{
    struct PrettyFormatter
        : Formatter
    {
        using Formatter::Formatter;

        constexpr static auto name = "pretty";

        struct Options
        {
            explicit Options(const nlohmann::json& formatOptions);

            const bool includeAttachments;
            const bool includeFeatureLine;
            const bool includeRuleLine;
            const bool useStatusIcon;
            const helper::Theme theme;
        };

    private:
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
        void PrintStepArgument(const cucumber::messages::pickle_step& pickleStep, std::size_t scenarioIndent, const helper::Theme& theme);
        void PrintAmbiguousStep(const cucumber::messages::test_step_finished& testStepFinished, const cucumber::messages::test_step& testStep, std::size_t scenarioIndent);

        void PrintGherkinLine(const std::string& title, const std::optional<std::string>& location, std::size_t indent, std::size_t maxContentLength);

        Options options{ formatOptions.contains(name) ? formatOptions.at(name) : nlohmann::json::object() };

        std::map<std::string, helper::GherkinScenarioMap> testCaseStartedIdToScenarioMap;

        std::map<std::string, std::size_t> maxContentLengthByTestCaseStartedId;
        std::map<std::string, std::size_t> scenarioIndentByTestCaseStartedId;

        std::set<const cucumber::messages::feature*> printedFeatureUris;
        std::set<const cucumber::messages::rule*> printedRuleIds;
    };
}

#endif
