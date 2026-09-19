#include "cucumber_cpp/library/runtime/NestedTestCaseRunner.hpp"
#include "cucumber/messages/PickleDocString.hpp"
#include "cucumber/messages/PickleTable.hpp"
#include "cucumber/messages/StepMatchArgumentsList.hpp"
#include "cucumber/messages/TestStep.hpp"
#include "cucumber/messages/TestStepResultStatus.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/support/MatchStepDefinitions.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/NestedTestCaseRunnerError.hpp"
#include "cucumber_cpp/library/util/TestStepResult.hpp"
#include "cucumber_cpp/library/util/TestStepResultStatus.hpp"
#include "cucumber_cpp/library/util/TestStepStarted.hpp"
#include "cucumber_cpp/library/util/TransformDocString.hpp"
#include "cucumber_cpp/library/util/TransformStepMatchArgumentsList.hpp"
#include "cucumber_cpp/library/util/TransformTable.hpp"
#include "cucumber_cpp/library/util/TransformTestStepResult.hpp"
#include <cstddef>
#include <cucumber/messages/TestStepResult.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace cucumber_cpp::library::runtime
{
    namespace
    {
        cucumber::messages::TestStep Assemble(const std::string& step, const support::SupportCodeLibrary& supportCodeLibrary, const util::TestStepStarted& testStepStarted)
        {
            cucumber::messages::TestStep testStep;
            testStep.id = testStepStarted.testStepId;
            testStep.stepDefinitionIds = std::vector<std::string>{};
            testStep.stepMatchArgumentsLists = std::vector<cucumber::messages::StepMatchArgumentsList>{};

            support::MatchStepDefinitions(testStep, supportCodeLibrary.stepRegistry.StepDefinitions(), step);

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

        void Run(std::size_t nesting, const std::string& step, const cucumber::messages::TestStep& testStep, const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, Context& testCaseContext, const util::TestStepStarted& testStepStarted, const std::optional<cucumber::messages::PickleTable>& dataTable, const std::optional<cucumber::messages::PickleDocString>& docString) // NOSONAR: cohesive step runner
        {
            auto stepDefinitions = (*testStep.stepDefinitionIds) | std::views::transform([&supportCodeLibrary](const std::string& id) -> const support::StepRegistry::Definition&
                                                                       {
                                                                           return supportCodeLibrary.stepRegistry.GetDefinitionById(id);
                                                                       });

            if (testStep.stepDefinitionIds->empty())
            {
                cucumber::messages::TestStepResult status{};
                status.status = cucumber::messages::TestStepResultStatus::UNDEFINED;
                throw util::NestedTestCaseRunnerError{ .nesting = nesting, .status = status, .text = step };
            }
            else if (testStep.stepDefinitionIds->size() > 1)
            {
                cucumber::messages::TestStepResult status{};
                status.message = "Ambiguous step definitions";
                status.status = cucumber::messages::TestStepResultStatus::AMBIGUOUS;
                throw util::NestedTestCaseRunnerError{ .nesting = nesting, .status = status, .text = step };
            }
            else
            {
                const auto& definition = stepDefinitions.front();
                NestedTestCaseRunner nestedTestCaseRunner{ nesting, supportCodeLibrary, broadcaster, testCaseContext, testStepStarted };
                const util::BodyFactory bodyFactory = [&nestedTestCaseRunner, &definition, &broadcaster, &testCaseContext, &testStepStarted, &dataTable, &docString](util::TestStepResult& testStepResult)
                {
                    return definition.factory(testStepResult, nestedTestCaseRunner, broadcaster, testCaseContext, testStepStarted, util::TransformTable(dataTable), util::TransformDocString(docString));
                };
                Invoke(nesting, step, bodyFactory, testStep.stepMatchArgumentsLists->front());
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
