#include "cucumber_cpp/library/assemble/AssembleTestSuites.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/step_match_arguments_list.hpp"
#include "cucumber/messages/test_case.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <list>
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
        const std::list<support::PickleSource>& sourcedPickles,
        cucumber::gherkin::id_generator_ptr idGenerator)
    {
        std::vector<std::string> testUris;
        std::map<std::string, AssembledTestSuite> assembledTestSuiteMap;

        for (const auto& pickleSource : sourcedPickles)
        {
            cucumber::messages::test_case testCase{
                .id = idGenerator->next_id(),
                .pickle_id = pickleSource.pickle->id,
                .test_steps = {},
                .test_run_started_id = std::make_optional<std::string>(testRunStartedId)
            };

            testCase.test_steps.reserve(pickleSource.pickle->steps.size() * 2); // steps + hooks

            for (const auto& hookId : supportCodeLibrary.hookRegistry.FindIds(HookType::before, pickleSource.pickle->tags))
                testCase.test_steps.emplace_back(hookId, idGenerator->next_id());

            for (const auto& step : pickleSource.pickle->steps)
            {
                // auto definitions = supportCodeLibrary.stepRegistry.FindDefinitions(step.text);
                const auto& stepDefinitions = supportCodeLibrary.stepRegistry.StepDefinitions();

                std::vector<std::string> stepDefinitionIds;
                std::vector<cucumber::messages::step_match_arguments_list> stepMatchArgumentsLists;

                for (const auto& definition : stepDefinitions)
                {
                    const auto match = std::visit(cucumber_expression::MatchVisitor{ step.text }, definition.regex);
                    if (match)
                    {
                        stepDefinitionIds.push_back(definition.id);
                        auto& argumentList = stepMatchArgumentsLists.emplace_back();
                        for (const auto& result : *match)
                            argumentList.step_match_arguments.emplace_back(result.Group(), result.Name());
                    }
                }

                testCase.test_steps.emplace_back(
                    std::nullopt,
                    idGenerator->next_id(),
                    step.id,
                    stepDefinitionIds,
                    stepMatchArgumentsLists);
            }

            for (const auto& hookId : supportCodeLibrary.hookRegistry.FindIds(HookType::after, pickleSource.pickle->tags) | std::views::reverse)
                testCase.test_steps.emplace_back(hookId, idGenerator->next_id());

            broadcaster.BroadcastEvent(cucumber::messages::envelope{ .test_case = testCase });

            if (!assembledTestSuiteMap.contains(pickleSource.gherkinDocument->uri.value()))
            {
                testUris.emplace_back(pickleSource.gherkinDocument->uri.value());
                assembledTestSuiteMap.emplace(pickleSource.gherkinDocument->uri.value(), *pickleSource.gherkinDocument);
            }

            assembledTestSuiteMap.at(pickleSource.gherkinDocument->uri.value()).testCases.emplace_back(*pickleSource.pickle, testCase);
        }

        std::vector<AssembledTestSuite> assembledTestSuites;
        assembledTestSuites.reserve(assembledTestSuiteMap.size());

        for (auto uri : testUris)
            assembledTestSuites.emplace_back(std::move(assembledTestSuiteMap.at(uri)));

        return assembledTestSuites;
    }
}
