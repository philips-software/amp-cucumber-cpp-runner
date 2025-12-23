#include "cucumber_cpp/library/formatter/helper/GherkinDocumentParser.hpp"
#include "cucumber/messages/background.hpp"
#include "cucumber/messages/feature_child.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/rule.hpp"
#include "cucumber/messages/rule_child.hpp"
#include "cucumber/messages/scenario.hpp"
#include "cucumber/messages/step.hpp"
#include <ranges>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::formatter::helper
{
    namespace
    {
        std::vector<cucumber::messages::step> ExtractStep(const std::variant<cucumber::messages::scenario, cucumber::messages::background> tests)
        {
            return std::visit([](const auto& item)
                {
                    return item.steps;
                },
                tests);
        }

        std::vector<cucumber::messages::scenario> ExtractScenarioFromRuleChild(const cucumber::messages::rule_child& child)
        {
            if (child.scenario)
                return { *child.scenario };
            return {};
        }

        std::vector<cucumber::messages::rule> ExtractRulesFeatureChild(const cucumber::messages::feature_child& child)
        {
            if (child.rule)
                return { *child.rule };
            return {};
        }

        std::vector<cucumber::messages::scenario> ExtractScenarioContainers(const cucumber::messages::feature_child& child)
        {
            if (child.rule)
            {
                std::vector<cucumber::messages::scenario> result;
                for (const auto& scenario : child.rule->children | std::views::transform(ExtractScenarioFromRuleChild) | std::views::join)
                    result.push_back(scenario);
                return result;
            }

            if (child.scenario)
                return { *child.scenario };

            return {};
        }

        std::variant<cucumber::messages::scenario, cucumber::messages::background> ExtractRuleContainers(const cucumber::messages::rule_child& child)
        {
            if (child.background)
                return *child.background;
            else
                return *child.scenario;
        }

        std::vector<std::variant<cucumber::messages::scenario, cucumber::messages::background>> ExtractStepContainers(const cucumber::messages::feature_child& child)
        {
            if (child.background)
                return { *child.background };
            if (child.rule)
            {
                auto iter = child.rule->children | std::views::transform(ExtractRuleContainers);
                return { iter.begin(), iter.end() };
            }

            return { *child.scenario };
        }
    }

    GherkinStepMap GetGherkinStepMap(const cucumber::messages::gherkin_document& gherkinDocument)
    {
        auto steps = gherkinDocument.feature->children |
                     std::views::transform(ExtractStepContainers) | std::views::join |
                     std::views::transform(ExtractStep) | std::views::join;

        GherkinStepMap map;

        for (const auto& step : steps)
            map.emplace(step.id, step);

        return map;
    }

    GherkinScenarioMap GetGherkinScenarioMap(const cucumber::messages::gherkin_document& gherkinDocument)
    {
        GherkinScenarioMap map;

        for (const auto& scenario : gherkinDocument.feature->children | std::views::transform(ExtractScenarioContainers) | std::views::join)
            map.emplace(scenario.id, scenario);

        return map;
    }

    GherkinScenarioLocationMap GetGherkinScenarioLocationMap(const cucumber::messages::gherkin_document& gherkinDocument)
    {
        GherkinScenarioLocationMap locationMap;
        GherkinScenarioMap scenarioMap = GetGherkinScenarioMap(gherkinDocument);

        for (const auto& [id, scenario] : scenarioMap)
        {
            locationMap.emplace(id, scenario.location);
            for (const auto& example : scenario.examples)
                for (const auto& tableRow : example.table_body)
                    locationMap.emplace(tableRow.id, tableRow.location);
        }

        return locationMap;
    }
}
