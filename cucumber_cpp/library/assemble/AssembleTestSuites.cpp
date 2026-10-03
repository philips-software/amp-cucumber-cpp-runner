#include "cucumber_cpp/library/assemble/AssembleTestSuites.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/StepMatchArgumentsList.hpp"
#include "cucumber/messages/TestCase.hpp"
#include "cucumber/messages/TestStep.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/support/HookRegistry.hpp"
#include "cucumber_cpp/library/support/MatchStepDefinitions.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include "cucumber_cpp/library/util/TransformPickleTag.hpp"
#include <cucumber/messages/PickleStep.hpp>
#include <list>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::assemble
{
    namespace
    {
        void AssembleSteps(const support::SupportCodeLibrary& supportCodeLibrary, const support::PickleSource& pickleSource, cucumber::messages::TestCase& testCase, cucumber::gherkin::IdGeneratorBase& idGenerator)
        {
            const auto& stepDefinitions = supportCodeLibrary.stepRegistry.StepDefinitions();

            for (const auto& pickleStep : pickleSource.pickle->steps)
            {
                cucumber::messages::TestStep testStep;
                testStep.id = idGenerator.NextId();
                testStep.pickleStepId = pickleStep.id;
                testStep.stepDefinitionIds = std::vector<std::string>{};
                testStep.stepMatchArgumentsLists = std::vector<cucumber::messages::StepMatchArgumentsList>{};

                support::MatchStepDefinitions(testCase.testSteps.emplace_back(std::move(testStep)), stepDefinitions, pickleStep.text);
            }
        }

        void AssembleTestSteps(const support::SupportCodeLibrary& supportCodeLibrary, const support::PickleSource& pickleSource, cucumber::messages::TestCase& testCase, cucumber::gherkin::IdGeneratorBase& idGenerator)
        {
            auto beforeHooks = supportCodeLibrary.hookRegistry.FindIds(util::HookType::before, util::TransformPickleTags(pickleSource.pickle->tags));
            auto afterHooks = supportCodeLibrary.hookRegistry.FindIds(util::HookType::after, util::TransformPickleTags(pickleSource.pickle->tags));

            testCase.testSteps.reserve(beforeHooks.size() + pickleSource.pickle->steps.size() + afterHooks.size());

            for (const auto& hookId : beforeHooks)
            {
                cucumber::messages::TestStep testStep;
                testStep.hookId = hookId;
                testStep.id = idGenerator.NextId();
                testCase.testSteps.push_back(testStep);
            }

            AssembleSteps(supportCodeLibrary, pickleSource, testCase, idGenerator);

            for (const auto& hookId : afterHooks | std::views::reverse)
            {
                cucumber::messages::TestStep testStep;
                testStep.hookId = hookId;
                testStep.id = idGenerator.NextId();
                testCase.testSteps.push_back(testStep);
            }
        }
    }

    std::vector<AssembledTestSuite> AssembleTestSuites(const support::SupportCodeLibrary& supportCodeLibrary,
        std::string_view testRunStartedId,
        util::Broadcaster& broadcaster,
        const std::list<support::PickleSource>& sourcedPickles,
        cucumber::gherkin::IdGeneratorBase& idGenerator)
    {
        std::list<std::string> testUris;
        std::map<std::string, AssembledTestSuite, std::less<>> assembledTestSuiteMap;

        for (const auto& pickleSource : sourcedPickles)
        {
            cucumber::messages::TestCase testCase;
            testCase.id = idGenerator.NextId();
            testCase.pickleId = pickleSource.pickle->id;
            testCase.testRunStartedId = std::make_optional<std::string>(testRunStartedId);

            AssembleTestSteps(supportCodeLibrary, pickleSource, testCase, idGenerator);

            broadcaster.BroadcastEvent([&testCase](cucumber::messages::Envelope& envelope)
                {
                    envelope.testCase = testCase;
                });

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
