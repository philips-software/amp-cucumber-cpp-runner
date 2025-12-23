#ifndef LIBRARY_QUERY_HPP
#define LIBRARY_QUERY_HPP

#include "cucumber/messages/attachment.hpp"
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
#include "cucumber/messages/step_match_arguments_list.hpp"
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
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber/messages/undefined_parameter_type.hpp"
#include <cstdint>
#include <forward_list>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library
{
    struct Lineage
    {
        Lineage operator+(std::shared_ptr<const cucumber::messages::gherkin_document> gherkinDocument) const;
        Lineage operator+(std::shared_ptr<const cucumber::messages::feature> feature) const;
        Lineage operator+(std::shared_ptr<const cucumber::messages::rule> rule) const;
        Lineage operator+(std::shared_ptr<const cucumber::messages::scenario> scenario) const;
        Lineage operator+(std::shared_ptr<const cucumber::messages::examples> examples) const;
        Lineage operator+(std::shared_ptr<const cucumber::messages::table_row> tableRow) const;
        Lineage operator+(std::uint32_t featureIndex) const;

        std::string GetUniqueFeatureName() const;
        std::string GetScenarioAndOrRuleName() const;

        std::shared_ptr<const cucumber::messages::gherkin_document> gherkinDocument;
        std::shared_ptr<const cucumber::messages::feature> feature;
        std::shared_ptr<const cucumber::messages::rule> rule;
        std::shared_ptr<const cucumber::messages::scenario> scenario;
        std::shared_ptr<const cucumber::messages::examples> examples;
        std::shared_ptr<const cucumber::messages::table_row> tableRow;

        std::uint32_t featureIndex{ 0 };
    };

    struct Query
    {
        Query& operator+=(const cucumber::messages::envelope& envelope);

        auto GetPickles() const
        {
            return pickleById | std::views::values;
        }

        const Lineage& FindLineageByPickle(const cucumber::messages::pickle& pickle) const;
        const Lineage& FindLineageByUri(const std::string&) const;

        const cucumber::messages::parameter_type& FindParameterTypeById(const std::string& id) const;
        const cucumber::messages::parameter_type& FindParameterTypeByName(const std::string& name) const;
        bool ContainsParameterTypeByName(const std::string& name) const;

        const cucumber::messages::test_case& FindTestCaseBy(const cucumber::messages::test_case_started& testCaseStarted) const;
        const cucumber::messages::test_case& FindTestCaseById(const std::string& id) const;

        const cucumber::messages::pickle& FindPickleBy(const cucumber::messages::test_case_started& testCaseStarted) const;
        const cucumber::messages::pickle& FindPickleById(const std::string& id) const;

        const cucumber::messages::pickle_step* FindPickleStepBy(const cucumber::messages::test_step& testStep) const;
        const cucumber::messages::pickle_step& FindPickleStepById(const std::string& id) const;

        const cucumber::messages::test_step& FindTestStepBy(const cucumber::messages::test_step_finished& testStepFinished) const;

        const cucumber::messages::step&
        FindStepBy(const cucumber::messages::pickle_step& pickleStep) const;

        const cucumber::messages::step_definition& FindStepDefinitionById(const std::string& id) const;

        const cucumber::messages::location& FindLocationOf(const cucumber::messages::pickle& pickle) const;

        const std::map<std::string, cucumber::messages::test_case_started, std::less<>>& TestCaseStarted() const;
        const std::map<std::string, cucumber::messages::test_case_finished, std::less<>>& TestCaseFinishedByTestCaseStartedId() const;

    private:
        void
        operator+=(const cucumber::messages::gherkin_document& gherkinDocument);
        void operator+=(const cucumber::messages::pickle& pickle);
        void operator+=(const cucumber::messages::hook& hook);
        void operator+=(const cucumber::messages::step_definition& stepDefinition);
        void operator+=(const cucumber::messages::test_run_started& testRunStarted);
        void operator+=(const cucumber::messages::test_run_hook_started& testRunHookStarted);
        void operator+=(const cucumber::messages::test_run_hook_finished& testRunHookFinished);
        void operator+=(const cucumber::messages::test_case& testCase);
        void operator+=(const cucumber::messages::test_case_started& testCaseStarted);
        void operator+=(const cucumber::messages::test_step_started& testStepStarted);
        void operator+=(const cucumber::messages::attachment& attachment);
        void operator+=(const cucumber::messages::test_step_finished& testStepFinished);
        void operator+=(const cucumber::messages::test_case_finished& testCaseFinished);
        void operator+=(const cucumber::messages::test_run_finished& testRunFinished);
        void operator+=(const cucumber::messages::suggestion& suggestion);
        void operator+=(const cucumber::messages::undefined_parameter_type& undefinedParameterType);

        void operator+=(std::pair<const cucumber::messages::feature&, Lineage> feature);
        void operator+=(std::pair<const cucumber::messages::scenario&, Lineage> scenario);
        void operator+=(std::pair<const cucumber::messages::rule&, Lineage> rule);
        void operator+=(std::span<const cucumber::messages::step> steps);

        void operator+=(const cucumber::messages::parameter_type& parameterType);

        std::map<std::string, std::uint32_t, std::less<>> featureCountByName;

        std::forward_list<cucumber::messages::test_step_result> testStepResults;
        std::map<std::string, std::forward_list<cucumber::messages::test_step_result*>, std::less<>> testStepResultByPickleId;
        std::map<std::string, std::forward_list<cucumber::messages::test_step_result*>, std::less<>> testStepResultsByPickleStepId;
        std::map<std::string, std::forward_list<cucumber::messages::test_step_result*>, std::less<>> testStepResultsbyTestStepId;

        std::map<std::string, cucumber::messages::test_case, std::less<>> testCaseById;
        std::map<std::string, cucumber::messages::test_case&, std::less<>> testCaseByPickleId;

        std::map<std::string, std::string, std::less<>> pickleIdByTestStepId;
        std::map<std::string, std::string, std::less<>> pickleStepIdByTestStepId;
        std::map<std::string, std::forward_list<std::string>, std::less<>> testStepIdsByPickleStepId;
        std::map<std::string, cucumber::messages::hook, std::less<>> hooksById;
        std::forward_list<cucumber::messages::attachment> attachments;
        std::map<std::string, std::forward_list<cucumber::messages::attachment*>, std::less<>> attachmentsByTestStepId;
        std::map<std::string, std::forward_list<cucumber::messages::attachment*>, std::less<>> attachmentsByTestCaseStartedId;
        // std::map<std::string, std::vector<std::shared_ptr<const cucumber::messages::attachment>>, std::less<>> attachmentsByTestRunHookStartedId;

        std::map<std::string, std::vector<cucumber::messages::step_match_arguments_list>, std::less<>> stepMatchArgumentsListsByPickleStepId;

        std::unique_ptr<const cucumber::messages::meta> meta;
        std::unique_ptr<const cucumber::messages::test_run_started> testRunStarted;
        std::unique_ptr<const cucumber::messages::test_run_finished> testRunFinished;

        std::map<std::string, cucumber::messages::test_case_started, std::less<>> testCaseStartedById;
        std::map<std::string, cucumber::messages::test_case_finished, std::less<>> testCaseFinishedByTestCaseStartedId;

        std::map<std::string, Lineage, std::less<>> lineageById;
        std::map<std::string, Lineage, std::less<>> lineageByUri;

        std::map<std::string, cucumber::messages::step, std::less<>> stepById;
        std::map<std::string, cucumber::messages::pickle, std::less<>> pickleById;
        std::map<std::string, cucumber::messages::pickle_step, std::less<>> pickleStepById;
        std::map<std::string, cucumber::messages::step_definition, std::less<>> stepDefinitionById;
        std::map<std::string, cucumber::messages::test_step, std::less<>> testStepById;
        std::map<std::string, cucumber::messages::test_run_hook_started, std::less<>> testRunHookStartedById;
        std::map<std::string, cucumber::messages::test_run_hook_finished, std::less<>> testRunHookFinishedByTestRunHookStartedId;
        std::map<std::string, std::forward_list<cucumber::messages::test_step_started>, std::less<>> testStepStartedByTestCaseStartedId;
        std::map<std::string, std::forward_list<cucumber::messages::test_step_finished>, std::less<>> testStepFinishedByTestCaseStartedId;

        std::map<std::string, std::forward_list<cucumber::messages::suggestion>, std::less<>> suggestionsByPickleStepId;
        std::forward_list<cucumber::messages::undefined_parameter_type> undefinedParameterTypes;

        std::map<std::string, cucumber::messages::parameter_type, std::less<>> parameterTypeById;
        std::map<std::string, cucumber::messages::parameter_type&, std::less<>> parameterTypeByName;
    };
}

#endif
