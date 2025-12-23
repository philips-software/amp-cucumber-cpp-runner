#include "cucumber_cpp/library/assemble/AssembleTestSuites.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/step_match_arguments_list.hpp"
#include "cucumber/messages/test_case.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
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
    namespace
    {
        auto TransformToMatch(const std::string& text)
        {
            return [&text](const StepRegistry::Definition& definition) -> std::pair<std::string, std::optional<std::vector<cucumber_expression::Argument>>>
            {
                const auto match = std::visit(cucumber_expression::MatchVisitor{ text }, definition.regex);
                return { definition.id, match };
            };
        }

        bool HasMatch(const std::pair<std::string, std::optional<std::vector<cucumber_expression::Argument>>>& pair)
        {
            return pair.second.has_value();
        }
    }

    std::vector<AssembledTestSuite> AssembleTestSuites(support::SupportCodeLibrary supportCodeLibrary,
        std::string_view testRunStartedId,
        util::Broadcaster& broadcaster,
        const std::list<support::PickleSource>& sourcedPickles,
        cucumber::gherkin::id_generator_ptr idGenerator)
    {
        std::list<std::string> testUris;
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
                const auto& stepDefinitions = supportCodeLibrary.stepRegistry.StepDefinitions();

                auto& testStep = testCase.test_steps.emplace_back(
                    std::nullopt,
                    idGenerator->next_id(),
                    step.id,
                    std::vector<std::string>{},
                    std::vector<cucumber::messages::step_match_arguments_list>{});

                for (const auto& [id, match] : stepDefinitions |
                                                   std::views::transform(TransformToMatch(step.text)) |
                                                   std::views::filter(HasMatch))
                {
                    testStep.step_definition_ids.value().push_back(id);
                    auto& argumentList = testStep.step_match_arguments_lists.value().emplace_back();
                    for (const auto& result : *match)
                        argumentList.step_match_arguments.emplace_back(result.Group(), result.Name().empty() ? std::nullopt : std::make_optional(result.Name()));
                }
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
