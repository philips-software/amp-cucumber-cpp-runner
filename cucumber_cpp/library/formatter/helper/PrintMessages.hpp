#ifndef HELPER_PRINT_MESSAGES_HPP
#define HELPER_PRINT_MESSAGES_HPP

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
#include "cucumber/query/Query.hpp"
#include "cucumber_cpp/library/formatter/helper/Theme.hpp"
#include <cstddef>
#include <memory>
#include <optional>
#include <ostream>
#include <string>

namespace cucumber_cpp::library::formatter::helper
{

    static constexpr auto gherkinIndentLength = 2;
    static constexpr auto stepArgumentIndentLength = 2;
    static constexpr auto attachmentIndentLength = 4;
    static constexpr auto errorIndentLength = 4;

    void PrintFeatureLine(std::ostream& stream, const cucumber::messages::Feature& feature, const Theme& theme);
    void PrintRuleLine(std::ostream& stream, const cucumber::messages::Rule& rule, const Theme& theme);
    void PrintTags(std::ostream& stream, const cucumber::messages::Pickle& pickle, std::size_t scenarioIndent, const Theme& theme);
    void PrintScenarioLine(std::ostream& stream, const cucumber::messages::Pickle& pickle, const cucumber::messages::Scenario& scenario, std::size_t scenarioIndent, std::size_t maxContentLength, const Theme& theme);
    void PrintScenarioAttemptLine(std::ostream& stream, const cucumber::messages::Pickle& pickle, std::size_t attempt, bool retry, const cucumber::messages::Scenario& scenario, std::size_t scenarioIndent, std::size_t maxContentLength, const Theme& theme);
    void PrintHookLine(std::ostream& stream, const cucumber::messages::TestStepFinished& testStepFinished, const cucumber::messages::Hook& hook, std::size_t scenarioIndent, std::size_t maxContentLength, bool isBeforeHook, bool useStatusIcon, const Theme& theme);
    void PrintStepLine(std::ostream& stream, const cucumber::messages::TestStepFinished& testStepFinished, const cucumber::messages::TestStep& testStep, const cucumber::messages::PickleStep& pickleStep, const cucumber::messages::Step& step, const cucumber::messages::StepDefinition* stepDefinition, std::size_t scenarioIndent, std::size_t maxContentLength, bool useStatusIcon, const Theme& theme);
    void PrintStepArgument(std::ostream& stream, const cucumber::messages::PickleStep& pickleStep, std::size_t scenarioIndent, bool useStatusIcon, const helper::Theme& theme);
    void PrintAmbiguousStep(std::ostream& stream, const cucumber::query::Query& query, const cucumber::messages::TestStepFinished& testStepFinished, const std::shared_ptr<const cucumber::messages::TestStep>& testStep, std::size_t scenarioIndent, bool useStatusIcon, const Theme& theme);
    void PrintError(std::ostream& stream, const cucumber::messages::TestStepFinished& testStepFinished, std::size_t scenarioIndent, bool useStatusIcon, const Theme& theme);
    void PrintGherkinLine(std::ostream& stream, const std::string& title, const std::optional<std::string>& location, std::size_t indent, std::size_t maxContentLength, const Theme& theme);
    void PrintAttachment(std::ostream& stream, const cucumber::messages::Attachment& attachment, std::size_t scenarioIndent, bool useStatusIcon, const Theme& theme);
}

#endif
