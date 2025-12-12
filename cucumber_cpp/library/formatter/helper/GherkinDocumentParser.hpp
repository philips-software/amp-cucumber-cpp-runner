#ifndef HELPER_GHERKIN_DOCUMENT_PARSER_HPP
#define HELPER_GHERKIN_DOCUMENT_PARSER_HPP

#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/rule.hpp"
#include "cucumber/messages/scenario.hpp"
#include "cucumber/messages/step.hpp"
#include <map>
#include <string>

namespace cucumber_cpp::library::formatter::helper
{
    using GherkinStepMap = std::map<std::string, cucumber::messages::step>;
    using GherkinScenarioRuleMap = std::map<std::string, cucumber::messages::rule>;
    using GherkinScenarioMap = std::map<std::string, cucumber::messages::scenario>;
    using GherkinScenarioLocationMap = std::map<std::string, cucumber::messages::location>;

    GherkinStepMap GetGherkinStepMap(const cucumber::messages::gherkin_document& gherkinDocument);
    GherkinScenarioRuleMap GetGherkinScenarioRuleMap(const cucumber::messages::gherkin_document& gherkinDocument);
    GherkinScenarioMap GetGherkinScenarioMap(const cucumber::messages::gherkin_document& gherkinDocument);
    GherkinScenarioLocationMap GetGherkinScenarioLocationMap(const cucumber::messages::gherkin_document& gherkinDocument);
}

#endif
