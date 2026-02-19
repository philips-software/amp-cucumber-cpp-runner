#include "cucumber_cpp/library/runtime/NestedTestCaseRunner.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/pickle_doc_string.hpp"
#include "cucumber/messages/pickle_table.hpp"
#include "cucumber/messages/step_match_arguments_list.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/support/Body.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/util/ArgumentGroupToMessageGroup.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::runtime
{
    namespace
    {
        auto TransformToMatch(const std::string& text)
        {
            return [&text](const support::StepRegistry::Definition& definition) -> std::pair<std::string, std::optional<std::vector<cucumber_expression::Argument>>>
            {
                const auto match = std::visit(cucumber_expression::MatchVisitor{ text }, definition.regex);
                return { definition.id, match };
            };
        }

        bool HasMatch(const std::pair<std::string, std::optional<std::vector<cucumber_expression::Argument>>>& pair)
        {
            return pair.second.has_value();
        }

        cucumber::messages::test_step Assemble(const std::string& step, const support::SupportCodeLibrary& supportCodeLibrary, const cucumber::messages::test_step_started& testStepStarted)
        {
            cucumber::messages::test_step testStep{
                .id = testStepStarted.test_step_id,
                .step_definition_ids = std::vector<std::string>{},
                .step_match_arguments_lists = std::vector<cucumber::messages::step_match_arguments_list>{}
            };

            const auto& stepDefinitions = supportCodeLibrary.stepRegistry.StepDefinitions();

            for (const auto& [id, match] : stepDefinitions |
                                               std::views::transform(TransformToMatch(step)) |
                                               std::views::filter(HasMatch))
            {
                testStep.step_definition_ids.value().push_back(id);
                auto& argumentList = testStep.step_match_arguments_lists.value().emplace_back();
                for (const auto& result : *match)
                    argumentList.step_match_arguments.emplace_back(util::ArgumentGroupToMessageGroup(result.Group()), result.Name().empty() ? std::nullopt : std::make_optional(result.Name()));
            }

            return testStep;
        }

        void Invoke(std::size_t nesting, const std::string& step, std::unique_ptr<support::Body> body, const cucumber::messages::step_match_arguments_list& args)
        {
            const auto status = body->ExecuteAndCatchExceptions(args);
            if (status.status != cucumber::messages::test_step_result_status::PASSED)
                throw NestedTestCaseRunnerError{ nesting, status, step };
        }

        void Run(std::size_t nesting, const std::string& step, const cucumber::messages::test_step& testStep, const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, Context& testCaseContext, const cucumber::messages::test_step_started& testStepStarted, const std::optional<cucumber::messages::pickle_table>& dataTable, const std::optional<cucumber::messages::pickle_doc_string>& docString)
        {
            auto stepDefinitions = (*testStep.step_definition_ids) | std::views::transform([&supportCodeLibrary](const std::string& id)
                                                                         {
                                                                             return supportCodeLibrary.stepRegistry.GetDefinitionById(id);
                                                                         });

            if (testStep.step_definition_ids->size() == 0)
                throw NestedTestCaseRunnerError{ nesting, {
                                                              .duration = cucumber::messages::duration{},
                                                              .status = cucumber::messages::test_step_result_status::UNDEFINED,
                                                          },
                    step };

            else if (testStep.step_definition_ids->size() > 1)
                throw NestedTestCaseRunnerError{ nesting, {
                                                              .duration = cucumber::messages::duration{},
                                                              .message = "Ambiguous step definitions",
                                                              .status = cucumber::messages::test_step_result_status::AMBIGUOUS,
                                                          },
                    step };
            else
            {
                const auto& definition = stepDefinitions.front();
                Invoke(nesting, step, definition.factory(NestedTestCaseRunner{ nesting, supportCodeLibrary, broadcaster, testCaseContext, testStepStarted }, broadcaster, testCaseContext, testStepStarted, dataTable, docString), testStep.step_match_arguments_lists->front());
            }
        }
    }

    NestedTestCaseRunner::NestedTestCaseRunner(std::size_t nesting, const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, Context& testCaseContext, const cucumber::messages::test_step_started& testStepStarted)
        : nesting{ nesting + 1 }
        , supportCodeLibrary{ supportCodeLibrary }
        , broadcaster{ broadcaster }
        , testCaseContext{ testCaseContext }
        , testStepStarted{ testStepStarted }
    {}

    void NestedTestCaseRunner::Step(const std::string& step) const
    {
        Step(step, std::nullopt, std::nullopt);
    }

    void NestedTestCaseRunner::Step(const std::string& step, const std::optional<cucumber::messages::pickle_doc_string>& docString) const
    {
        Step(step, std::nullopt, docString);
    }

    void NestedTestCaseRunner::Step(const std::string& step, const std::optional<cucumber::messages::pickle_table>& dataTable) const
    {
        Step(step, dataTable, std::nullopt);
    }

    void NestedTestCaseRunner::Step(const std::string& step, const std::optional<cucumber::messages::pickle_table>& dataTable, const std::optional<cucumber::messages::pickle_doc_string>& docString) const
    {
        const auto testStep = Assemble(step, supportCodeLibrary, testStepStarted);
        Run(nesting, step, testStep, supportCodeLibrary, broadcaster, testCaseContext, testStepStarted, dataTable, docString);
    }
}
