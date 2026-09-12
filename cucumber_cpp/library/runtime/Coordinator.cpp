#include "cucumber_cpp/library/runtime/Coordinator.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/TestRunFinished.hpp"
#include "cucumber/messages/TestRunStarted.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/MakeShared.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include <memory>
#include <string>
#include <utility>

namespace cucumber_cpp::library::runtime
{
    Coordinator::Coordinator(std::string testRunStartedId,
        util::Broadcaster& broadcaster,
        cucumber::gherkin::IdGeneratorPtr idGenerator,
        std::unique_ptr<support::RuntimeAdapter>&& runtimeAdapter,
        support::SupportCodeLibrary& supportCodeLibrary)
        : testRunStartedId{ std::move(testRunStartedId) }
        , broadcaster{ broadcaster }
        , idGenerator{ std::move(idGenerator) }
        , runtimeAdapter{ std::move(runtimeAdapter) }
        , supportCodeLibrary{ supportCodeLibrary }
    {}

    bool Coordinator::Run()
    {
        broadcaster.BroadcastEvent([this](cucumber::messages::Envelope& envelope)
            {
                cucumber::messages::TestRunStarted testRunStarted;
                testRunStarted.timestamp = util::TimestampNow();
                testRunStarted.id = std::string{ testRunStartedId };
                envelope.testRunStarted = std::move(testRunStarted);
            });

        const auto success = runtimeAdapter->Run();

        broadcaster.BroadcastEvent([this, &success](cucumber::messages::Envelope& envelope)
            {
                cucumber::messages::TestRunFinished testRunFinished;
                testRunFinished.success = success;
                testRunFinished.timestamp = util::TimestampNow();
                testRunFinished.testRunStartedId = std::string{ testRunStartedId };
                envelope.testRunFinished = std::move(testRunFinished);
            });

        return success;
    }
}
