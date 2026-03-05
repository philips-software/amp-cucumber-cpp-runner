#ifndef HELPER_PRINT_MESSAGES_HPP
#define HELPER_PRINT_MESSAGES_HPP

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
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include "cucumber_cpp/library/query/Query.hpp"
#include <cstddef>
#include <optional>
#include <ostream>
#include <string>

namespace cucumber_cpp::library::formatter::helper
{

    static constexpr auto gherkinIndentLength = 2;
    static constexpr auto stepArgumentIndentLength = 2;
    static constexpr auto attachmentIndentLength = 4;
    static constexpr auto errorIndentLength = 4;

    void PrintFeatureLine(std::ostream& stream, const cucumber::messages::feature& feature, const Theme& theme);
    void PrintRuleLine(std::ostream& stream, const cucumber::messages::rule& rule, const Theme& theme);
    void PrintTags(std::ostream& stream, const cucumber::messages::pickle& pickle, std::size_t scenarioIndent, const Theme& theme);
    void PrintScenarioLine(std::ostream& stream, const cucumber::messages::pickle& pickle, const cucumber::messages::scenario& scenario, std::size_t scenarioIndent, std::size_t maxContentLength, const Theme& theme);
    void PrintScenarioAttemptLine(std::ostream& stream, const cucumber::messages::pickle& pickle, std::size_t attempt, bool retry, const cucumber::messages::scenario& scenario, std::size_t scenarioIndent, std::size_t maxContentLength, const Theme& theme);
    void PrintHookLine(std::ostream& stream, const cucumber::messages::test_step_finished& testStepFinished, const cucumber::messages::hook& hook, std::size_t scenarioIndent, std::size_t maxContentLength, bool isBeforeHook, bool useStatusIcon, const Theme& theme);
    void PrintStepLine(std::ostream& stream, const cucumber::messages::test_step_finished& testStepFinished, const cucumber::messages::test_step& testStep, const cucumber::messages::pickle_step& pickleStep, const cucumber::messages::step& step, const cucumber::messages::step_definition* stepDefinition, std::size_t scenarioIndent, std::size_t maxContentLength, bool useStatusIcon, const Theme& theme);
    void PrintStepArgument(std::ostream& stream, const cucumber::messages::pickle_step& pickleStep, std::size_t scenarioIndent, bool useStatusIcon, const helper::Theme& theme);
    void PrintAmbiguousStep(std::ostream& stream, const query::Query& query, const cucumber::messages::test_step_finished& testStepFinished, const cucumber::messages::test_step& testStep, std::size_t scenarioIndent, bool useStatusIcon, const Theme& theme);
    void PrintError(std::ostream& stream, const cucumber::messages::test_step_finished& testStepFinished, std::size_t scenarioIndent, bool useStatusIcon, const Theme& theme);
    void PrintGherkinLine(std::ostream& stream, const std::string& title, const std::optional<std::string>& location, std::size_t indent, std::size_t maxContentLength, const Theme& theme);
    void PrintAttachment(std::ostream& stream, const cucumber::messages::attachment& attachment, std::size_t scenarioIndent, bool useStatusIcon, const Theme& theme);
}

#endif
