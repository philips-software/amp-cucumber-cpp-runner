#include "cucumber_cpp/library/Query.hpp"
#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/background.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/examples.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/hook.hpp"
#include "cucumber/messages/location.hpp"
#include "cucumber/messages/meta.hpp"
#include "cucumber/messages/parameter_type.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber/messages/rule.hpp"
#include "cucumber/messages/scenario.hpp"
#include "cucumber/messages/step.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/suggestion.hpp"
#include "cucumber/messages/table_row.hpp"
#include "cucumber/messages/test_case.hpp"
#include "cucumber/messages/test_case_finished.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_run_finished.hpp"
#include "cucumber/messages/test_run_hook_finished.hpp"
#include "cucumber/messages/test_run_hook_started.hpp"
#include "cucumber/messages/test_run_started.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber/messages/undefined_parameter_type.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <cstdint>
#include <format>
#include <functional>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <utility>

namespace cucumber_cpp::library
{
    Lineage operator+(Lineage lineage, std::shared_ptr<const cucumber::messages::gherkin_document> gherkinDocument)
    {
        lineage.gherkinDocument = gherkinDocument;
        return std::move(lineage);
    }

    Lineage operator+(Lineage lineage, std::shared_ptr<const cucumber::messages::feature> feature)
    {
        lineage.feature = feature;
        return std::move(lineage);
    }

    Lineage operator+(Lineage lineage, std::shared_ptr<const cucumber::messages::rule> rule)
    {
        lineage.rule = rule;
        return std::move(lineage);
    }

    Lineage operator+(Lineage lineage, std::shared_ptr<const cucumber::messages::scenario> scenario)
    {
        lineage.scenario = scenario;
        return std::move(lineage);
    }

    Lineage operator+(Lineage lineage, std::shared_ptr<const cucumber::messages::examples> examples)
    {
        lineage.examples = examples;
        return std::move(lineage);
    }

    Lineage operator+(Lineage lineage, std::shared_ptr<const cucumber::messages::table_row> tableRow)
    {
        lineage.tableRow = tableRow;
        return std::move(lineage);
    }

    Lineage operator+(Lineage lineage, std::uint32_t featureIndex)
    {
        lineage.featureIndex = featureIndex;
        return std::move(lineage);
    }

    std::string Lineage::GetUniqueFeatureName() const
    {
        return std::format("{}/{}", feature->name, featureIndex);
    }

    std::string Lineage::GetScenarioAndOrRuleName() const
    {
        if (rule)
            return std::format("{}/{}", rule->name, scenario->name);
        return scenario->name;
    }

    Query::Query(util::Broadcaster& broadcaster)
        : util::Listener{ broadcaster, [this](const cucumber::messages::envelope& envelope)
            {
                operator+=(envelope);
            } }
    {}

    const Lineage& Query::FindLineageByPickle(const cucumber::messages::pickle& pickle) const
    {
        return lineageById.at(pickle.ast_node_ids[0]);
    }

    const Lineage& Query::FindLineageByUri(const std::string& uri) const
    {
        return lineageByUri.at(uri);
    }

    const cucumber::messages::parameter_type& Query::FindParameterTypeById(const std::string& id) const
    {
        return parameterTypeById.at(id);
    }

    const cucumber::messages::parameter_type& Query::FindParameterTypeByName(const std::string& name) const
    {
        return parameterTypeByName.at(name);
    }

    bool Query::ContainsParameterTypeByName(const std::string& name) const
    {
        return parameterTypeByName.contains(name);
    }

    const cucumber::messages::test_case& Query::FindTestCaseBy(const cucumber::messages::test_case_started& testCaseStarted) const
    {
        return testCaseById.at(testCaseStarted.test_case_id);
    }

    const cucumber::messages::test_case& Query::FindTestCaseById(const std::string& id) const
    {
        return testCaseById.at(id);
    }

    const cucumber::messages::pickle& Query::FindPickleBy(const cucumber::messages::test_case_started& testCaseStarted) const
    {
        const auto& testCase = FindTestCaseById(testCaseStarted.test_case_id);
        return FindPickleById(testCase.pickle_id);
    }

    const cucumber::messages::pickle& Query::FindPickleById(const std::string& id) const
    {
        return pickleById.at(id);
    }

    const cucumber::messages::pickle_step* Query::FindPickleStepBy(const cucumber::messages::test_step& testStep) const
    {
        if (!testStep.pickle_step_id.has_value())
            return nullptr;
        return &pickleStepById.at(testStep.pickle_step_id.value());
    }

    const cucumber::messages::pickle_step& Query::FindPickleStepById(const std::string& id) const
    {
        return pickleStepById.at(id);
    }

    const cucumber::messages::test_step& Query::FindTestStepBy(const cucumber::messages::test_step_finished& testStepFinished) const
    {
        return testStepById.at(testStepFinished.test_step_id);
    }

    const cucumber::messages::step& Query::FindStepBy(const cucumber::messages::pickle_step& pickleStep) const
    {
        return stepById.at(pickleStep.ast_node_ids[0]);
    }

    const cucumber::messages::step_definition& Query::FindStepDefinitionById(const std::string& id) const
    {
        return stepDefinitionById.at(id);
    }

    const cucumber::messages::location& Query::FindLocationOf(const cucumber::messages::pickle& pickle) const
    {
        const auto& lineage = FindLineageByUri(pickle.uri);
        // if (lineage.examples)
        return lineage.scenario->location;
    }

    const std::map<std::string, cucumber::messages::test_case_started, std::less<>>& Query::TestCaseStarted() const
    {
        return testCaseStartedById;
    }

    const std::map<std::string, cucumber::messages::test_case_finished, std::less<>>& Query::TestCaseFinishedByTestCaseStartedId() const
    {
        return testCaseFinishedByTestCaseStartedId;
    }

    void Query::operator+=(const cucumber::messages::envelope& envelope)
    {
        if (envelope.meta)
            meta = std::make_unique<cucumber::messages::meta>(*envelope.meta);

        if (envelope.gherkin_document)
            *this += *envelope.gherkin_document;

        if (envelope.pickle)
            *this += *envelope.pickle;

        if (envelope.hook)
            *this += *envelope.hook;

        if (envelope.step_definition)
            *this += *envelope.step_definition;

        if (envelope.test_run_started)
            *this += *envelope.test_run_started;

        if (envelope.test_run_hook_started)
            *this += *envelope.test_run_hook_started;

        if (envelope.test_run_hook_finished)
            *this += *envelope.test_run_hook_finished;

        if (envelope.test_case)
            *this += *envelope.test_case;

        if (envelope.test_case_started)
            *this += *envelope.test_case_started;

        if (envelope.test_step_started)
            *this += *envelope.test_step_started;

        if (envelope.attachment)
            *this += *envelope.attachment;

        if (envelope.test_step_finished)
            *this += *envelope.test_step_finished;

        if (envelope.test_case_finished)
            *this += *envelope.test_case_finished;

        if (envelope.test_run_finished)
            *this += *envelope.test_run_finished;

        if (envelope.suggestion)
            *this += *envelope.suggestion;

        if (envelope.undefined_parameter_type)
            *this += *envelope.undefined_parameter_type;

        if (envelope.parameter_type)
            *this += *envelope.parameter_type;

        BroadcastEvent(envelope);
    }

    void Query::operator+=(const cucumber::messages::gherkin_document& gherkinDocument)
    {
        if (gherkinDocument.feature)
            *this += { *gherkinDocument.feature, Lineage{ std::make_unique<cucumber::messages::gherkin_document>(gherkinDocument) } };
    }

    void Query::operator+=(const cucumber::messages::pickle& pickle)
    {
        pickleById.try_emplace(pickle.id, pickle);
        for (const auto& pickleStep : pickle.steps)
            pickleStepById.try_emplace(pickleStep.id, pickleStep);
    }

    void Query::operator+=(const cucumber::messages::hook& hook)
    {
        hooksById.try_emplace(hook.id, hook);
    }

    void Query::operator+=(const cucumber::messages::step_definition& stepDefinition)
    {
        stepDefinitionById.try_emplace(stepDefinition.id, stepDefinition);
    }

    void Query::operator+=(const cucumber::messages::test_run_started& testRunStarted)
    {
        this->testRunStarted = std::make_unique<cucumber::messages::test_run_started>(testRunStarted);
    }

    void Query::operator+=(const cucumber::messages::test_run_hook_started& testRunHookStarted)
    {
        testRunHookStartedById.try_emplace(testRunHookStarted.id, testRunHookStarted);
    }

    void Query::operator+=(const cucumber::messages::test_run_hook_finished& testRunHookFinished)
    {
        testRunHookFinishedByTestRunHookStartedId.try_emplace(testRunHookFinished.test_run_hook_started_id, testRunHookFinished);
    }

    void Query::operator+=(const cucumber::messages::test_case& testCase)
    {
        auto& testCaseRef = testCaseById.try_emplace(testCase.id, testCase).first->second;
        testCaseByPickleId.try_emplace(testCase.pickle_id, testCaseRef);

        for (const auto& testStep : testCase.test_steps)
        {
            testStepById.try_emplace(testStep.id, testStep);
            pickleIdByTestStepId.try_emplace(testStep.id, testCase.pickle_id);

            if (testStep.pickle_step_id)
            {
                pickleStepIdByTestStepId[testStep.id] = testStep.pickle_step_id.value();
                testStepIdsByPickleStepId[testStep.pickle_step_id.value()].push_front(testStep.id);

                if (testStep.step_match_arguments_lists)
                    stepMatchArgumentsListsByPickleStepId[testStep.pickle_step_id.value()] = testStep.step_match_arguments_lists.value();
            }
        }
    }

    void Query::operator+=(const cucumber::messages::test_case_started& testCaseStarted)
    {
        testCaseStartedById.try_emplace(testCaseStarted.id, testCaseStarted);

        /* reset data? https://github.dev/cucumber/query/blob/f31732e5972c1815614f1d83928a7065e3080dc4/javascript/src/Query.ts#L249 */
    }

    void Query::operator+=(const cucumber::messages::test_step_started& testStepStarted)
    {
        testStepStartedByTestCaseStartedId[testStepStarted.test_case_started_id].push_front(testStepStarted);
    }

    void Query::operator+=(const cucumber::messages::attachment& attachment)
    {
        auto* ptr = &attachments.emplace_front(attachment);
        if (attachment.test_step_id)
            attachmentsByTestStepId[attachment.test_step_id.value()].push_front(ptr);
        if (attachment.test_case_started_id)
            attachmentsByTestCaseStartedId[attachment.test_case_started_id.value()].push_front(ptr);
    }

    void Query::operator+=(const cucumber::messages::test_step_finished& testStepFinished)
    {
        auto* testStepResultPtr = &testStepResults.emplace_front(testStepFinished.test_step_result);

        testStepFinishedByTestCaseStartedId[testStepFinished.test_case_started_id].push_front(testStepFinished);

        const auto& pickleId = pickleIdByTestStepId.at(testStepFinished.test_step_id);
        testStepResultByPickleId[pickleId].push_front(testStepResultPtr);

        const auto& testStep = testStepById.at(testStepFinished.test_step_id);
        testStepResultsbyTestStepId[testStep.id].push_front(testStepResultPtr);
        if (testStep.pickle_step_id)
            testStepResultsByPickleStepId[testStep.pickle_step_id.value()].push_front(testStepResultPtr);
    }

    void Query::operator+=(const cucumber::messages::test_case_finished& testCaseFinished)
    {
        testCaseFinishedByTestCaseStartedId.try_emplace(testCaseFinished.test_case_started_id, testCaseFinished);
    }

    void Query::operator+=(const cucumber::messages::test_run_finished& testRunFinished)
    {
        this->testRunFinished = std::make_unique<cucumber::messages::test_run_finished>(testRunFinished);
    }

    void Query::operator+=(const cucumber::messages::suggestion& suggestion)
    {
        suggestionsByPickleStepId[suggestion.pickle_step_id].push_front(suggestion);
    }

    void Query::operator+=(const cucumber::messages::undefined_parameter_type& undefinedParameterType)
    {
        undefinedParameterTypes.emplace_front(undefinedParameterType);
    }

    void Query::operator+=(std::pair<const cucumber::messages::feature&, Lineage> feature)
    {
        auto featurePtr = std::make_shared<cucumber::messages::feature>(feature.first);

        ++featureCountByName[feature.first.name];

        auto linaege = feature.second + featurePtr + featureCountByName[feature.first.name];
        lineageByUri[*linaege.gherkinDocument->uri] = linaege;

        for (const auto& child : feature.first.children)
        {
            if (child.background)
                *this += child.background->steps;
            if (child.scenario)
                *this += { *child.scenario, linaege };
            if (child.rule)
                *this += { *child.rule, linaege };
        }
    }

    void Query::operator+=(std::pair<const cucumber::messages::scenario&, Lineage> scenario)
    {
        auto scenarioPtr = std::make_shared<cucumber::messages::scenario>(scenario.first);
        lineageByUri[*scenario.second.gherkinDocument->uri] = scenario.second;

        lineageById[scenarioPtr->id] = scenario.second + scenarioPtr;

        for (const auto& examples : scenario.first.examples)
        {
            auto examplesPtr = std::make_shared<cucumber::messages::examples>(examples);

            lineageById[examples.id] = scenario.second +
                                       scenarioPtr +
                                       examplesPtr;

            for (const auto& tableRow : examples.table_body)
            {
                auto tableRowPtr = std::make_shared<cucumber::messages::table_row>(tableRow);

                lineageById[tableRow.id] = scenario.second +
                                           scenarioPtr +
                                           examplesPtr +
                                           tableRowPtr;
            }
        }

        *this += scenarioPtr->steps;
    }

    void Query::operator+=(std::pair<const cucumber::messages::rule&, Lineage> rule)
    {
        auto rulePtr = std::make_shared<cucumber::messages::rule>(rule.first);

        for (const auto& child : rule.first.children)
        {
            if (child.background)
                *this += child.background->steps;
            if (child.scenario)
                *this += { *child.scenario, rule.second + rulePtr };
        }
    }

    void Query::operator+=(std::span<const cucumber::messages::step> steps)
    {
        for (const auto& step : steps)
            stepById.try_emplace(step.id, step);
    }

    void Query::operator+=(const cucumber::messages::parameter_type& parameterType)
    {
        auto& ref = parameterTypeById.try_emplace(parameterType.id, parameterType).first->second;
        parameterTypeByName.try_emplace(parameterType.name, ref);
    }
}
