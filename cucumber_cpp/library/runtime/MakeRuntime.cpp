#include "cucumber_cpp/library/runtime/MakeRuntime.hpp"
#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/runtime/Coordinator.hpp"
#include "cucumber_cpp/library/runtime/SerialRuntimeAdapter.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <list>
#include <memory>
#include <string>

namespace cucumber_cpp::library::runtime
{
    std::unique_ptr<support::RuntimeAdapter> MakeAdapter(const support::RunOptions::Runtime& options, std::string testRunStartedId, util::Broadcaster& broadcaster, const std::list<support::PickleSource>& sourcedPickles, support::SupportCodeLibrary& supportCodeLibrary, cucumber::gherkin::IdGeneratorPtr idGenerator, Context& programContext)
    {
        return std::make_unique<runtime::SerialRuntimeAdapter>(
            testRunStartedId,
            broadcaster,
            idGenerator,
            sourcedPickles,
            options,
            supportCodeLibrary,
            programContext);
    }

    std::unique_ptr<support::Runtime> MakeRuntime(const support::RunOptions::Runtime& options, util::Broadcaster& broadcaster, const std::list<support::PickleSource>& sourcedPickles, support::SupportCodeLibrary& supportCodeLibrary, cucumber::gherkin::IdGeneratorPtr idGenerator, Context& programContext)
    {
        const auto testRunStartedId{ idGenerator->NextId() };
        return std::make_unique<runtime::Coordinator>(
            testRunStartedId,
            broadcaster,
            idGenerator,
            MakeAdapter(options, testRunStartedId, broadcaster, sourcedPickles, supportCodeLibrary, idGenerator, programContext),
            supportCodeLibrary);
    }
}
