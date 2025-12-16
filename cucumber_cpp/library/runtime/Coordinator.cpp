#include "cucumber_cpp/library/runtime/Coordinator.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/test_run_finished.hpp"
#include "cucumber/messages/test_run_started.hpp"
#include "cucumber_cpp/library/assemble/AssembleTestSuites.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Timestamp.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <memory>
#include <span>
#include <string>
#include <utility>

namespace cucumber_cpp::library::runtime
{
    Coordinator::Coordinator(std::string testRunStartedId,
        util::Broadcaster& broadcaster,
        cucumber::gherkin::id_generator_ptr idGenerator,
        std::unique_ptr<support::RuntimeAdapter>&& runtimeAdapter,
        support::SupportCodeLibrary supportCodeLibrary)
        : testRunStartedId{ testRunStartedId }
        , broadcaster{ broadcaster }
        , idGenerator{ idGenerator }
        , runtimeAdapter{ std::move(runtimeAdapter) }
        , supportCodeLibrary{ supportCodeLibrary }
    {}

    bool Coordinator::Run()
    {
        broadcaster.BroadcastEvent({ .test_run_started = cucumber::messages::test_run_started{
                                         .timestamp = support::TimestampNow(),
                                         .id = std::string{ testRunStartedId },
                                     } });

        const auto success = runtimeAdapter->Run();

        broadcaster.BroadcastEvent({ .test_run_finished = cucumber::messages::test_run_finished{
                                         .success = success,
                                         .timestamp = support::TimestampNow(),
                                         .test_run_started_id = std::string{ testRunStartedId },
                                     } });

        return success;
    }
}
