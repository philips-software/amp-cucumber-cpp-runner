#include "cucumber_cpp/library/runtime/NestedTestCaseRunner.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/group.hpp"
#include "cucumber/messages/pickle_doc_string.hpp"
#include "cucumber/messages/pickle_table.hpp"
#include "cucumber/messages/step_match_argument.hpp"
#include "cucumber/messages/step_match_arguments_list.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/util/ArgumentGroupToMessageGroup.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/NestedTestCaseRunnerError.hpp"
#include "cucumber_cpp/library/util/TestStepResultStatus.hpp"
#include "cucumber_cpp/library/util/TestStepStarted.hpp"
#include "cucumber_cpp/library/util/TransformDocString.hpp"
#include "cucumber_cpp/library/util/TransformTable.hpp"
#include "cucumber_cpp/library/util/TransformTestStepResult.hpp"
#include <cstddef>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::runtime
{
    namespace
    {
        std::optional<std::string> ToString(const cucumber::messages::group& group)
        {
            return group.value;
        }

        cucumber_expression::ConvertFunctionArg GroupToArgumentGroup(const cucumber::messages::group& group)
        {
            if (group.children.empty())
                return { group.value };

            auto strings = group.children | std::views::transform(ToString);

            return { strings.begin(), strings.end() };
        }

        util::Argument ToArgument(const cucumber::messages::step_match_argument& argument)
        {
            return {
                .converterName = argument.parameter_type_name.value_or(""),
                .converterArgs = GroupToArgumentGroup(argument.group),
            };
        }

        util::ExecuteArgs StepMatchArgumentsListToExecuteArgs(const cucumber::messages::step_match_arguments_list& args)
        {
            auto strings = args.step_match_arguments | std::views::transform(ToArgument);
            return { strings.begin(), strings.end() };
        }

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

        cucumber::messages::test_step Assemble(const std::string& step, const support::SupportCodeLibrary& supportCodeLibrary, const util::TestStepStarted& testStepStarted)
        {
            cucumber::messages::test_step testStep{
                .id = testStepStarted.testStepId,
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

        void Invoke(std::size_t nesting, const std::string& step, std::unique_ptr<util::Body> body, const cucumber::messages::step_match_arguments_list& args)
        {
            const auto status = body->ExecuteAndCatchExceptions(StepMatchArgumentsListToExecuteArgs(args));
            if (status.status != util::TestStepResultStatus::PASSED)
                throw util::NestedTestCaseRunnerError{
                    .nesting = nesting,
                    .status = util::TransformTestStepResult(status),
                    .text = step,
                };
        }

        void Run(std::size_t nesting, const std::string& step, const cucumber::messages::test_step& testStep, const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, Context& testCaseContext, const util::TestStepStarted& testStepStarted, const std::optional<cucumber::messages::pickle_table>& dataTable, const std::optional<cucumber::messages::pickle_doc_string>& docString)
        {
            auto stepDefinitions = (*testStep.step_definition_ids) | std::views::transform([&supportCodeLibrary](const std::string& id)
                                                                         {
                                                                             return supportCodeLibrary.stepRegistry.GetDefinitionById(id);
                                                                         });

            if (testStep.step_definition_ids->size() == 0)
                throw util::NestedTestCaseRunnerError{ .nesting = nesting, .status = {
                                                                               .duration = cucumber::messages::duration{},
                                                                               .status = cucumber::messages::test_step_result_status::UNDEFINED,
                                                                           },
                    .text = step };

            else if (testStep.step_definition_ids->size() > 1)
                throw util::NestedTestCaseRunnerError{ .nesting = nesting, .status = {
                                                                               .duration = cucumber::messages::duration{},
                                                                               .message = "Ambiguous step definitions",
                                                                               .status = cucumber::messages::test_step_result_status::AMBIGUOUS,
                                                                           },
                    .text = step };
            else
            {
                const auto& definition = stepDefinitions.front();
                Invoke(nesting, step, definition.factory(NestedTestCaseRunner{ nesting, supportCodeLibrary, broadcaster, testCaseContext, testStepStarted }, broadcaster, testCaseContext, testStepStarted, util::TransformTable(dataTable), util::TransformDocString(docString)), testStep.step_match_arguments_lists->front());
            }
        }
    }

    NestedTestCaseRunner::NestedTestCaseRunner(std::size_t nesting, const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, Context& testCaseContext, util::TestStepStarted testStepStarted)
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
