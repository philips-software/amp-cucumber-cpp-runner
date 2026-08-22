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
        broadcaster.BroadcastEvent(util::MakeShared(cucumber::messages::TestRunStarted{
            .timestamp = util::MakeShared(util::TimestampNow()),
            .id = std::string{ testRunStartedId },
        }));

        const auto success = runtimeAdapter->Run();

        broadcaster.BroadcastEvent(util::MakeShared(cucumber::messages::TestRunFinished{
            .success = success,
            .timestamp = util::MakeShared(util::TimestampNow()),
            .testRunStartedId = std::string{ testRunStartedId },
        }));

        return success;
    }
}
