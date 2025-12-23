#include "cucumber_cpp/library/runtime/SerialRuntimeAdapter.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/assemble/AssembleTestSuites.hpp"
#include "cucumber_cpp/library/runtime/Worker.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/GetWorstTestStepResult.hpp"
#include <list>
#include <span>
#include <string>

namespace cucumber_cpp::library::runtime
{
    SerialRuntimeAdapter::SerialRuntimeAdapter(std::string testRunStartedId,
        util::Broadcaster& broadcaster,
        cucumber::gherkin::id_generator_ptr idGenerator,
        const std::list<support::PickleSource>& sourcedPickles,
        const support::RunOptions::Runtime& options,
        support::SupportCodeLibrary& supportCodeLibrary,
        Context& programContext)
        : testRunStartedId{ testRunStartedId }
        , broadcaster{ broadcaster }
        , idGenerator{ idGenerator }
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

        if (util::GetWorstTestStepResult(beforeHookResults).status != cucumber::messages::test_step_result_status::PASSED)
            failing = true;

        if (!failing)
        {
            auto assembledTestSuites = assemble::AssembleTestSuites(supportCodeLibrary, testRunStartedId, broadcaster, sourcedPickles, idGenerator);

            for (const auto& assembledTestSuite : assembledTestSuites)
            {
                try
                {
                    const auto success = worker.RunTestSuite(assembledTestSuite, failing);
                    if (!success)
                        failing = true;
                }
                catch (...)
                {
                    failing = true;
                }
            }
        }

        const auto afterHookResults = worker.RunAfterAllHooks();

        if (util::GetWorstTestStepResult(afterHookResults).status != cucumber::messages::test_step_result_status::PASSED)
            failing = true;

        return !failing;
    }
}
