#include "cucumber_cpp/library/query/Query.hpp"
#include "cucumber/messages/attachment.hpp"
#include "cucumber/messages/background.hpp"
#include "cucumber/messages/duration.hpp"
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
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber/messages/undefined_parameter_type.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include <algorithm>
#include <cstddef>
#include <format>
#include <forward_list>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::query
{
    namespace
    {
        struct SortByStatus
        {
            bool operator()(const std::pair<const cucumber::messages::test_step_finished*, const cucumber::messages::test_step*>& a,
                const std::pair<const cucumber::messages::test_step_finished*, const cucumber::messages::test_step*>& b) const
            {
                const auto& statusA = a.first->test_step_result.status;
                const auto& statusB = b.first->test_step_result.status;
                return static_cast<int>(statusA) < static_cast<int>(statusB);
            }
        };
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

    std::string NamingStrategy::Reduce(const Lineage& lineage, const cucumber::messages::pickle& pickle)
    {
        std::vector<std::string> parts;
        parts.reserve(10);

        if (!lineage.feature)
            parts.emplace_back(lineage.feature->name);

        if (lineage.rule)
            parts.emplace_back(lineage.rule->name);

        if (lineage.scenario)
            parts.emplace_back(lineage.scenario->name);
        else
            parts.emplace_back(pickle.name);

        if (lineage.examples)
            parts.emplace_back(lineage.examples->name);

        if (parts.size() == 1)
            return parts.front();

        return std::accumulate(std::next(parts.begin()), parts.end(), parts.front(), [](const std::string& a, const std::string& b)
            {
                if (b.empty())
                    return a;
                else
                    return a + " - " + b;
            });
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

    std::list<const cucumber::messages::step_definition*> Query::FindStepDefinitionsById(const cucumber::messages::test_step& testStep) const
    {
        if (!testStep.step_definition_ids.has_value())
            return {};

        auto view = testStep.step_definition_ids.value() | std::views::transform([this](const std::string& id)
                                                               {
                                                                   return &FindStepDefinitionById(id);
                                                               });

        return { view.begin(), view.end() };
    }

    const cucumber::messages::location& Query::FindLocationOf(const cucumber::messages::pickle& pickle) const
    {
        const auto& lineage = FindLineageByUri(pickle.uri);
        if (lineage.tableRow)
            return lineage.tableRow->location;
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

    std::size_t Query::CountTestCasesStarted() const
    {
        return FindAllTestCaseStarted().size();
    }

    std::map<cucumber::messages::test_step_result_status, std::size_t, std::less<>> Query::CountMostSevereTestStepResultStatus() const
    {
        std::map<cucumber::messages::test_step_result_status, std::size_t, std::less<>> result{
            { cucumber::messages::test_step_result_status::UNKNOWN, 0 },
            { cucumber::messages::test_step_result_status::PASSED, 0 },
            { cucumber::messages::test_step_result_status::SKIPPED, 0 },
            { cucumber::messages::test_step_result_status::PENDING, 0 },
            { cucumber::messages::test_step_result_status::UNDEFINED, 0 },
            { cucumber::messages::test_step_result_status::AMBIGUOUS, 0 },
            { cucumber::messages::test_step_result_status::FAILED, 0 },
        };

        for (const auto* testCaseStarted : FindAllTestCaseStarted())
        {
            const auto testStepFinishedAndTestSteps = FindTestStepFinishedAndTestStepBy(*testCaseStarted);
            const auto [testStepFinished, testStep] = *std::ranges::max_element(testStepFinishedAndTestSteps, SortByStatus{});
            ++result[testStepFinished->test_step_result.status];
        }

        return result;
    }

    std::optional<const cucumber::messages::test_step_result*> Query::FindMostSevereTestStepResultBy(const cucumber::messages::test_case_started& testCaseStarted) const
    {
        auto list = FindTestStepFinishedAndTestStepBy(testCaseStarted);
        if (list.empty())
            return std::nullopt;

        list.sort(SortByStatus{});

        const auto [testStepFinished, testStep] = list.back();
        return &testStepFinished->test_step_result;
    }

    std::optional<const cucumber::messages::test_step_result*> Query::FindMostSevereTestStepResultBy(const cucumber::messages::test_case_finished& testCaseFinished) const
    {
        return FindMostSevereTestStepResultBy(testCaseStartedById.at(testCaseFinished.test_case_started_id));
    }

    std::list<const cucumber::messages::test_case_started*> Query::FindAllTestCaseStarted() const
    {
        auto view = testCaseStartedById |
                    std::views::values |
                    std::views::filter([this](const cucumber::messages::test_case_started& testCaseStarted)
                        {
                            return !testCaseFinishedByTestCaseStartedId.at(testCaseStarted.id).will_be_retried;
                        }) |
                    std::views::transform([](const cucumber::messages::test_case_started& testCaseStarted)
                        {
                            return &testCaseStarted;
                        });
        return { view.begin(), view.end() };
    }

    std::list<std::pair<const cucumber::messages::test_step_finished*, const cucumber::messages::test_step*>> Query::FindTestStepFinishedAndTestStepBy(const cucumber::messages::test_case_started& testCaseStarted) const
    {
        const auto& testStepsFinished = testStepFinishedByTestCaseStartedId.at(testCaseStarted.id);
        auto view = testStepsFinished | std::views::transform([this](const auto& testStepFinished)
                                            {
                                                return std::make_pair(&testStepFinished, &FindTestStepBy(testStepFinished));
                                            });
        return { view.begin(), view.end() };
    }

    const cucumber::messages::test_run_started& Query::FindTestRunStarted() const
    {
        return *testRunStarted;
    }

    cucumber::messages::duration Query::FindTestRunDuration() const
    {
        return testRunFinished->timestamp - testRunStarted->timestamp;
    }

    cucumber::messages::duration Query::FindTestCaseDurationBy(const cucumber::messages::test_case_started& testCaseStarted) const
    {
        return FindTestCaseDurationBy(testCaseStarted, testCaseFinishedByTestCaseStartedId.at(testCaseStarted.id));
    }

    cucumber::messages::duration Query::FindTestCaseDurationBy(const cucumber::messages::test_case_finished& testCaseFinished) const
    {
        return FindTestCaseDurationBy(testCaseStartedById.at(testCaseFinished.test_case_started_id), testCaseFinished);
    }

    cucumber::messages::duration Query::FindTestCaseDurationBy(const cucumber::messages::test_case_started& testCaseStarted, const cucumber::messages::test_case_finished& testCaseFinished) const
    {
        return testCaseFinished.timestamp - testCaseStarted.timestamp;
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
                testStepIdsByPickleStepId[testStep.pickle_step_id.value()].push_back(testStep.id);

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
        testStepStartedByTestCaseStartedId[testStepStarted.test_case_started_id].push_back(testStepStarted);
    }

    void Query::operator+=(const cucumber::messages::attachment& attachment)
    {
        auto* ptr = &attachments.emplace_front(attachment);
        if (attachment.test_step_id)
            attachmentsByTestStepId[attachment.test_step_id.value()].push_back(ptr);
        if (attachment.test_case_started_id)
            attachmentsByTestCaseStartedId[attachment.test_case_started_id.value()].push_back(ptr);
    }

    void Query::operator+=(const cucumber::messages::test_step_finished& testStepFinished)
    {
        auto* testStepResultPtr = &testStepResults.emplace_front(testStepFinished.test_step_result);

        testStepFinishedByTestCaseStartedId[testStepFinished.test_case_started_id].push_back(testStepFinished);

        const auto& pickleId = pickleIdByTestStepId.at(testStepFinished.test_step_id);
        testStepResultByPickleId[pickleId].push_back(testStepResultPtr);

        const auto& testStep = testStepById.at(testStepFinished.test_step_id);
        testStepResultsbyTestStepId[testStep.id].push_back(testStepResultPtr);
        if (testStep.pickle_step_id)
            testStepResultsByPickleStepId[testStep.pickle_step_id.value()].push_back(testStepResultPtr);
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
        suggestionsByPickleStepId[suggestion.pickle_step_id].push_back(suggestion);
    }

    void Query::operator+=(const cucumber::messages::undefined_parameter_type& undefinedParameterType)
    {
        undefinedParameterTypes.emplace_front(undefinedParameterType);
    }

    void Query::operator+=(const std::pair<const cucumber::messages::feature&, Lineage>& feature)
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

    void Query::operator+=(const std::pair<const cucumber::messages::scenario&, Lineage>& scenario)
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

    void Query::operator+=(const std::pair<const cucumber::messages::rule&, Lineage>& rule)
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
