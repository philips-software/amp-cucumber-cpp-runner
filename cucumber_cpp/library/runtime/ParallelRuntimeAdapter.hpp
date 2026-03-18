#ifndef RUNTIME_PARALLEL_RUNTIME_ADAPTER_HPP
#define RUNTIME_PARALLEL_RUNTIME_ADAPTER_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <coro/latch.hpp>
#include <coro/queue.hpp>
#include <coro/task.hpp>
#include <coro/thread_pool.hpp>
#include <list>
#include <memory>
#include <string>

namespace cucumber_cpp::library::runtime
{
    struct ParallelRuntimeAdapter : support::RuntimeAdapter
    {
        ParallelRuntimeAdapter(std::string testRunStartedId,
            util::Broadcaster& broadcaster,
            cucumber::gherkin::id_generator_ptr idGenerator,
            const std::list<support::PickleSource>& sourcedPickles,
            const support::RunOptions::Runtime& options,
            support::SupportCodeLibrary& supportCodeLibrary,
            Context& programContext);

        bool Run() override;

    private:
        std::string testRunStartedId;
        util::Broadcaster& broadcaster;
        cucumber::gherkin::id_generator_ptr idGenerator;
        const std::list<support::PickleSource>& sourcedPickles;
        const support::RunOptions::Runtime& options;
        support::SupportCodeLibrary& supportCodeLibrary;
        Context& programContext;

        std::unique_ptr<coro::thread_pool> threadPool;
    };
}

#endif
