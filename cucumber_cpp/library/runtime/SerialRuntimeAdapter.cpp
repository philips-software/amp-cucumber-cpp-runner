#include "cucumber_cpp/library/runtime/SerialRuntimeAdapter.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/assemble/AssembleTestSuites.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/runtime/Worker.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <span>
#include <string>

namespace cucumber_cpp::library::runtime
{
    SerialRuntimeAdapter::SerialRuntimeAdapter(std::string testRunStartedId,
        util::Broadcaster& broadcaster,
        cucumber::gherkin::id_generator_ptr idGenerator,
        std::span<const support::PickleSource> sourcedPickles,
        const support::RunOptions::Runtime& options,
        support::SupportCodeLibrary supportCodeLibrary,
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

        worker.RunBeforeAllHooks();

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

        worker.RunAfterAllHooks();

        return !failing;
    }
}
