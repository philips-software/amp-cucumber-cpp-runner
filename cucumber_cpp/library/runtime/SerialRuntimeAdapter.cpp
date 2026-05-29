#include "cucumber_cpp/library/runtime/SerialRuntimeAdapter.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/assemble/AssembleTestSuites.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/runtime/Worker.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/GetWorstTestStepResult.hpp"
#include <list>
#include <span>
#include <string>
#include <utility>

namespace cucumber_cpp::library::runtime
{
    namespace
    {
        bool IsFailing(cucumber::messages::test_step_result_status status, bool dryRun)
        {
            if (dryRun)
                return false;

            return status != cucumber::messages::test_step_result_status::PASSED;
        }
    }

    SerialRuntimeAdapter::SerialRuntimeAdapter(std::string testRunStartedId,
        util::Broadcaster& broadcaster,
        cucumber::gherkin::id_generator_ptr idGenerator,
        const std::list<support::PickleSource>& sourcedPickles,
        const support::RunOptions::Runtime& options,
        support::SupportCodeLibrary& supportCodeLibrary,
        Context& programContext)
        : testRunStartedId{ std::move(testRunStartedId) }
        , broadcaster{ broadcaster }
        , idGenerator{ std::move(idGenerator) }
        , sourcedPickles{ sourcedPickles }
        , options{ options }
        , supportCodeLibrary{ supportCodeLibrary }
        , programContext{ programContext }
    {}

    bool SerialRuntimeAdapter::Run()
    {
        bool failing = false;
        runtime::Worker worker{ testRunStartedId, broadcaster, idGenerator, options, supportCodeLibrary, programContext };

        const auto beforeHookResults = worker.RunBeforeAllHooks();

        if (IsFailing(util::GetWorstTestStepResult(beforeHookResults).status, options.dryRun))
            failing = true;

        if (!failing)
        {
            auto assembledTestSuites = assemble::AssembleTestSuites(supportCodeLibrary, testRunStartedId, broadcaster, sourcedPickles, idGenerator);

            for (const auto& assembledTestSuite : assembledTestSuites)
                try
                {
                    const auto success = RunTestSuite(worker, assembledTestSuite, failing);
                    if (!success)
                        failing = true;
                }
                catch (...)
                {
                    failing = true;
                }
        }

        const auto afterHookResults = worker.RunAfterAllHooks();

        if (IsFailing(util::GetWorstTestStepResult(afterHookResults).status, options.dryRun))
            failing = true;

        return !failing;
    }

    bool SerialRuntimeAdapter::RunTestSuite(runtime::Worker& worker, const assemble::AssembledTestSuite& assembledTestSuite, bool failing)
    {
        Context testSuiteContext{ &programContext };

        auto failed = false;

        if (options.featureHooks)
        {
            const auto beforeHookResults = worker.RunBeforeTestSuiteHooks(*assembledTestSuite.gherkinDocument.feature, testSuiteContext);

            if (IsFailing(util::GetWorstTestStepResult(beforeHookResults).status, options.dryRun))
                failing = true;
        }

        for (const auto& assembledTestCase : assembledTestSuite.testCases)
            failed |= !worker.RunTestCase(assembledTestSuite.gherkinDocument, assembledTestCase, testSuiteContext, failed || failing);

        if (options.featureHooks)
        {
            const auto afterHookResults = worker.RunAfterTestSuiteHooks(*assembledTestSuite.gherkinDocument.feature, testSuiteContext);

            if (IsFailing(util::GetWorstTestStepResult(afterHookResults).status, options.dryRun))
                failing = true;
        }

        return !failed;
    }
}
