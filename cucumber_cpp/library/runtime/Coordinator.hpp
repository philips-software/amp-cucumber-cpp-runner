#ifndef RUNTIME_COORDINATOR_HPP
#define RUNTIME_COORDINATOR_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <memory>
#include <span>
#include <string>

namespace cucumber_cpp::library::runtime
{
    struct Coordinator : support::Runtime
    {
        Coordinator(std::string testRunStartedId,
            util::Broadcaster& broadcaster,
            cucumber::gherkin::id_generator_ptr idGenerator,
            std::unique_ptr<support::RuntimeAdapter>&& runtimeAdapter,
            support::SupportCodeLibrary supportCodeLibrary);

        bool Run() override;

    private:
        std::string testRunStartedId;
        util::Broadcaster& broadcaster;
        cucumber::gherkin::id_generator_ptr idGenerator;
        std::unique_ptr<support::RuntimeAdapter> runtimeAdapter;
        support::SupportCodeLibrary supportCodeLibrary;
    };
}

#endif
