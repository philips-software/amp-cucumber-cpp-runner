#include "cucumber_cpp/library/runtime/NestedTestCaseRunner.hpp"
#include "cucumber/messages/Duration.hpp"
#include "cucumber/messages/PickleDocString.hpp"
#include "cucumber/messages/PickleTable.hpp"
#include "cucumber/messages/StepMatchArgumentsList.hpp"
#include "cucumber/messages/TestStep.hpp"
#include "cucumber/messages/TestStepResultStatus.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/util/ArgumentGroupToMessageGroup.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/NestedTestCaseRunnerError.hpp"
#include "cucumber_cpp/library/util/TestStepResultStatus.hpp"
#include "cucumber_cpp/library/util/TestStepStarted.hpp"
#include "cucumber_cpp/library/util/TransformDocString.hpp"
#include "cucumber_cpp/library/util/TransformStepMatchArgumentsList.hpp"
#include "cucumber_cpp/library/util/TransformTable.hpp"
#include "cucumber_cpp/library/util/TransformTestStepResult.hpp"
#include <cstddef>
#include <memory>
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

        cucumber::messages::TestStep Assemble(const std::string& step, const support::SupportCodeLibrary& supportCodeLibrary, const util::TestStepStarted& testStepStarted)
        {
            cucumber::messages::TestStep testStep{
                .id = testStepStarted.testStepId,
                .stepDefinitionIds = std::vector<std::string>{},
                .stepMatchArgumentsLists = std::vector<std::shared_ptr<cucumber::messages::StepMatchArgumentsList>>{}
            };

            const auto& stepDefinitions = supportCodeLibrary.stepRegistry.StepDefinitions();

            for (const auto& [id, match] : stepDefinitions |
                                               std::views::transform(TransformToMatch(step)) |
                                               std::views::filter(HasMatch))
            {
                testStep.stepDefinitionIds.value().push_back(id);
                auto& argumentList = testStep.stepMatchArgumentsLists.value().emplace_back(std::make_shared<cucumber::messages::StepMatchArgumentsList>());
                for (const auto& result : *match)
                    argumentList->stepMatchArguments.emplace_back(std::make_shared<cucumber::messages::StepMatchArgument>(cucumber::messages::StepMatchArgument{
                        .group = std::make_shared<cucumber::messages::Group>(util::ArgumentGroupToMessageGroup(result.Group())),
                        .parameterTypeName = result.Name().empty() ? std::nullopt : std::make_optional(result.Name()),
                    }));
            }

            return testStep;
        }

        void Invoke(std::size_t nesting, const std::string& step, const util::BodyFactory& bodyFactory, const cucumber::messages::StepMatchArgumentsList& args)
        {
            const auto status = util::ConstructAndExecute(bodyFactory, util::StepMatchArgumentsListToExecuteArgs(args));

            if (status.status != util::TestStepResultStatus::PASSED)
                throw util::NestedTestCaseRunnerError{
                    .nesting = nesting,
                    .status = util::TransformTestStepResult(status),
                    .text = step,
                };
        }

        void Run(std::size_t nesting, const std::string& step, const cucumber::messages::TestStep& testStep, const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, Context& testCaseContext, const util::TestStepStarted& testStepStarted, const std::optional<cucumber::messages::PickleTable>& dataTable, const std::optional<cucumber::messages::PickleDocString>& docString)
        {
            auto stepDefinitions = (*testStep.stepDefinitionIds) | std::views::transform([&supportCodeLibrary](const std::string& id)
                                                                       {
                                                                           return supportCodeLibrary.stepRegistry.GetDefinitionById(id);
                                                                       });

            if (testStep.stepDefinitionIds->empty())
                throw util::NestedTestCaseRunnerError{ .nesting = nesting, .status = {
                                                                               .duration = std::make_shared<cucumber::messages::Duration>(),
                                                                               .status = cucumber::messages::TestStepResultStatus::UNDEFINED,
                                                                           },
                    .text = step };

            else if (testStep.stepDefinitionIds->size() > 1)
                throw util::NestedTestCaseRunnerError{ .nesting = nesting, .status = {
                                                                               .duration = std::make_shared<cucumber::messages::Duration>(),
                                                                               .message = "Ambiguous step definitions",
                                                                               .status = cucumber::messages::TestStepResultStatus::AMBIGUOUS,
                                                                           },
                    .text = step };
            else
            {
                const auto& definition = stepDefinitions.front();
                NestedTestCaseRunner nestedTestCaseRunner{ nesting, supportCodeLibrary, broadcaster, testCaseContext, testStepStarted };
                const util::BodyFactory bodyFactory = [&nestedTestCaseRunner, &definition, &broadcaster, &testCaseContext, &testStepStarted, &dataTable, &docString](util::TestStepResult& testStepResult)
                {
                    return definition.factory(testStepResult, nestedTestCaseRunner, broadcaster, testCaseContext, testStepStarted, util::TransformTable(dataTable), util::TransformDocString(docString));
                };
                Invoke(nesting, step, bodyFactory, *testStep.stepMatchArgumentsLists->front());
            }
        }
    }

    NestedTestCaseRunner::NestedTestCaseRunner(std::size_t nesting, const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, Context& testCaseContext, util::TestStepStarted testStepStarted)
        : nesting{ nesting + 1 }
        , supportCodeLibrary{ supportCodeLibrary }
        , broadcaster{ broadcaster }
        , testCaseContext{ testCaseContext }
        , testStepStarted{ std::move(testStepStarted) }
    {}

    void NestedTestCaseRunner::Step(const std::string& step) const
    {
        Step(step, std::nullopt, std::nullopt);
    }

    void NestedTestCaseRunner::Step(const std::string& step, const std::optional<cucumber::messages::PickleDocString>& docString) const
    {
        Step(step, std::nullopt, docString);
    }

    void NestedTestCaseRunner::Step(const std::string& step, const std::optional<cucumber::messages::PickleTable>& dataTable) const
    {
        Step(step, dataTable, std::nullopt);
    }

    void NestedTestCaseRunner::Step(const std::string& step, const std::optional<cucumber::messages::PickleTable>& dataTable, const std::optional<cucumber::messages::PickleDocString>& docString) const
    {
        const auto testStep = Assemble(step, supportCodeLibrary, testStepStarted);
        Run(nesting, step, testStep, supportCodeLibrary, broadcaster, testCaseContext, testStepStarted, dataTable, docString);
    }
}
