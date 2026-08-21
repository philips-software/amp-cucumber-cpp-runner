#include "cucumber_cpp/library/assemble/AssembleTestSuites.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/StepMatchArgumentsList.hpp"
#include "cucumber/messages/TestCase.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/cucumber_expression/Argument.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/support/HookRegistry.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/ArgumentGroupToMessageGroup.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include "cucumber_cpp/library/util/TransformPickleTag.hpp"
#include <functional>
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

        void AssembleSteps(const support::SupportCodeLibrary& supportCodeLibrary, const support::PickleSource& pickleSource, cucumber::messages::TestCase& testCase, cucumber::gherkin::IdGeneratorPtr idGenerator)
        {
            for (const auto& step : pickleSource.pickle->steps)
            {
                const auto& stepDefinitions = supportCodeLibrary.stepRegistry.StepDefinitions();

                auto& testStep = testCase.testSteps.emplace_back(std::make_shared<cucumber::messages::TestStep>(cucumber::messages::TestStep{
                    .hookId = std::nullopt,
                    .id = idGenerator->NextId(),
                    .pickleStepId = step->id,
                    .stepDefinitionIds = std::vector<std::string>{},
                    .stepMatchArgumentsLists = std::vector<std::shared_ptr<cucumber::messages::StepMatchArgumentsList>>{},
                }));

                for (const auto& [id, match] : stepDefinitions |
                                                   std::views::transform(TransformToMatch(step->text)) |
                                                   std::views::filter(HasMatch))
                {
                    testStep->stepDefinitionIds.value().push_back(id);
                    auto& argumentList = testStep->stepMatchArgumentsLists.value().emplace_back(std::make_shared<cucumber::messages::StepMatchArgumentsList>());
                    for (const auto& result : *match)
                        argumentList->stepMatchArguments.emplace_back(std::make_shared<cucumber::messages::StepMatchArgument>(cucumber::messages::StepMatchArgument{
                            .group = std::make_shared<cucumber::messages::Group>(util::ArgumentGroupToMessageGroup(result.Group())),
                            .parameterTypeName = result.Name().empty() ? std::nullopt : std::make_optional(result.Name()),
                        }));
                }
            }
        }

        void AssembleTestSteps(const support::SupportCodeLibrary& supportCodeLibrary, const support::PickleSource& pickleSource, cucumber::messages::TestCase& testCase, cucumber::gherkin::IdGeneratorPtr idGenerator)
        {
            auto beforeHooks = supportCodeLibrary.hookRegistry.FindIds(util::HookType::before, util::TransformPickleTags((pickleSource.pickle->tags)));
            auto afterHooks = supportCodeLibrary.hookRegistry.FindIds(util::HookType::after, util::TransformPickleTags((pickleSource.pickle->tags)));

            testCase.testSteps.reserve(beforeHooks.size() + pickleSource.pickle->steps.size() + afterHooks.size());

            for (const auto& hookId : beforeHooks)
                testCase.testSteps.emplace_back(std::make_shared<cucumber::messages::TestStep>(cucumber::messages::TestStep{ .hookId = hookId, .id = idGenerator->NextId() }));

            AssembleSteps(supportCodeLibrary, pickleSource, testCase, idGenerator);

            for (const auto& hookId : afterHooks | std::views::reverse)
                testCase.testSteps.emplace_back(std::make_shared<cucumber::messages::TestStep>(cucumber::messages::TestStep{ .hookId = hookId, .id = idGenerator->NextId() }));
        }
    }

    std::vector<AssembledTestSuite> AssembleTestSuites(const support::SupportCodeLibrary& supportCodeLibrary,
        std::string_view testRunStartedId,
        util::Broadcaster& broadcaster,
        const std::list<support::PickleSource>& sourcedPickles,
        cucumber::gherkin::IdGeneratorPtr idGenerator)
    {
        std::list<std::string> testUris;
        std::map<std::string, AssembledTestSuite, std::less<>> assembledTestSuiteMap;

        for (const auto& pickleSource : sourcedPickles)
        {
            cucumber::messages::TestCase testCase{
                .id = idGenerator->NextId(),
                .pickleId = pickleSource.pickle->id,
                .testSteps = {},
                .testRunStartedId = std::make_optional<std::string>(testRunStartedId)
            };

            AssembleTestSteps(supportCodeLibrary, pickleSource, testCase, idGenerator);

            broadcaster.BroadcastEvent(cucumber::messages::Envelope{ .testCase = std::make_shared<cucumber::messages::TestCase>(testCase) });

            if (!assembledTestSuiteMap.contains(pickleSource.gherkinDocument->uri.value()))
            {
                testUris.emplace_back(pickleSource.gherkinDocument->uri.value());
                assembledTestSuiteMap.try_emplace(pickleSource.gherkinDocument->uri.value(), *pickleSource.gherkinDocument);
            }

            assembledTestSuiteMap.at(pickleSource.gherkinDocument->uri.value()).testCases.emplace_back(*pickleSource.pickle, testCase);
        }

        std::vector<AssembledTestSuite> assembledTestSuites;
        assembledTestSuites.reserve(assembledTestSuiteMap.size());

        for (const auto& uri : testUris)
            assembledTestSuites.emplace_back(std::move(assembledTestSuiteMap.at(uri)));

        return assembledTestSuites;
    }
}
