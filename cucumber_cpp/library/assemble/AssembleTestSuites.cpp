#include "cucumber_cpp/library/assemble/AssembleTestSuites.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/step_match_arguments_list.hpp"
#include "cucumber/messages/test_case.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <map>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::assemble
{
    std::vector<AssembledTestSuite> AssembleTestSuites(support::SupportCodeLibrary supportCodeLibrary,
        std::string_view testRunStartedId,
        util::Broadcaster& broadcaster,
        std::span<const support::PickleSource> sourcedPickles,
        cucumber::gherkin::id_generator_ptr idGenerator)
    {
        std::map<std::string, AssembledTestSuite> assembledTestSuiteMap;

        for (const auto& pickleSource : sourcedPickles)
        {
            std::vector<cucumber::messages::test_step> allSteps;
            allSteps.reserve(pickleSource.pickle->steps.size() * 2); // steps + hooks

            for (const auto& hookId : supportCodeLibrary.hookRegistry.FindIds(HookType::before, pickleSource.pickle->tags))
                allSteps.emplace_back(hookId, idGenerator->next_id());

            for (const auto& step : pickleSource.pickle->steps)
            {
                auto definitions = supportCodeLibrary.stepRegistry.FindDefinitions(step.text);
                const auto& stepDefinitions = supportCodeLibrary.stepRegistry.StepDefinitions();

                std::vector<std::string> stepDefinitionIds;
                std::vector<cucumber::messages::step_match_arguments_list> stepMatchArgumentsLists;

                for (const auto& [id, definition] : stepDefinitions)
                {
                    auto optionalStepMatchArgumentsList = std::visit(cucumber_expression::MatchArgumentsVisitor{ step.text }, definition.regex);

                    if (optionalStepMatchArgumentsList)
                    {
                        stepDefinitionIds.push_back(id);
                        stepMatchArgumentsLists.push_back(*optionalStepMatchArgumentsList);
                    }
                }

                allSteps.emplace_back(
                    std::nullopt,
                    idGenerator->next_id(),
                    step.id,
                    stepDefinitionIds,
                    stepMatchArgumentsLists);
            }

            for (const auto& hookId : supportCodeLibrary.hookRegistry.FindIds(HookType::after, pickleSource.pickle->tags))
                allSteps.emplace_back(hookId, idGenerator->next_id());

            cucumber::messages::test_case testCase{
                .id = idGenerator->next_id(),
                .pickle_id = pickleSource.pickle->id,
                .test_steps = std::move(allSteps),
                .test_run_started_id = std::make_optional<std::string>(testRunStartedId)
            };

            broadcaster.BroadcastEvent(cucumber::messages::envelope{ .test_case = testCase });

            if (!assembledTestSuiteMap.contains(pickleSource.gherkinDocument->uri.value()))
                assembledTestSuiteMap.emplace(pickleSource.gherkinDocument->uri.value(), *pickleSource.gherkinDocument);

            assembledTestSuiteMap.at(pickleSource.gherkinDocument->uri.value()).testCases.emplace_back(*pickleSource.pickle, testCase);
        }

        std::vector<AssembledTestSuite> assembledTestSuites;
        assembledTestSuites.reserve(assembledTestSuiteMap.size());

        for (auto assembledTestSuiteValues : assembledTestSuiteMap | std::views::values)
            assembledTestSuites.emplace_back(std::move(assembledTestSuiteValues));

        return assembledTestSuites;
    }
}
