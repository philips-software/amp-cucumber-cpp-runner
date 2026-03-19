#include "cucumber_cpp/library/runtime/ParallelRuntimeAdapter.hpp"
#include "coro/task.hpp"
#include "coro/thread_pool.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/assemble/AssembleTestSuites.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestCase.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/runtime/Worker.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/GetWorstTestStepResult.hpp"
#include <coro/latch.hpp>
#include <coro/queue.hpp>
#include <coro/sync_wait.hpp>
#include <coro/when_all.hpp>
#include <iterator>
#include <list>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

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

        coro::task<void> RunTestCase(std::unique_ptr<coro::thread_pool>& tp, coro::latch& tasksLatch, runtime::Worker& worker, const cucumber::messages::gherkin_document& gherkinDocument, const assemble::AssembledTestCase& assembledTestCase, Context& testSuiteContext, bool& failing)
        {
            co_await tp->schedule();

            try
            {
                failing |= !worker.RunTestCase(gherkinDocument, assembledTestCase, testSuiteContext, failing);
            }
            catch (...)
            {
                failing = true;
            }

            tasksLatch.count_down();

            co_return;
        }

        struct ParallelBroadcaster : util::Broadcaster
        {
            ParallelBroadcaster(util::Broadcaster& broadcaster, std::unique_ptr<coro::thread_pool>& threadPool)
                : broadcaster{ broadcaster }
                , threadPool{ threadPool }
            {
            }

            void AddListener(util::Listener* listener) override
            {
                broadcaster.AddListener(listener);
            }

            void RemoveListener(util::Listener* listener) override
            {
                broadcaster.RemoveListener(listener);
            }

            void BroadcastEvent(const cucumber::messages::envelope& envelope) override
            {
                coro::sync_wait(queue.emplace(envelope));
            }

            coro::task<void> BroadcastEventTask()
            {
                co_await threadPool->schedule();

                while (true)
                {
                    auto event = co_await queue.pop();

                    if (!event)
                        break;

                    broadcaster.BroadcastEvent(*event);
                }
            }

            coro::task<void> ShutdownTask(coro::latch& latch)
            {
                co_await threadPool->schedule();
                co_await latch;
                co_await queue.shutdown_drain(threadPool);
                co_return;
            }

        private:
            util::Broadcaster& broadcaster;
            std::unique_ptr<coro::thread_pool>& threadPool;

            coro::queue<cucumber::messages::envelope> queue;
        };
    }

    ParallelRuntimeAdapter::ParallelRuntimeAdapter(std::string testRunStartedId,
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
        , threadPool{ coro::thread_pool::make_unique(coro::thread_pool::options{ .thread_count = options.parallel }) }
    {
    }

    bool ParallelRuntimeAdapter::Run()
    {
        std::vector<coro::task<void>> tasks;
        bool failing = false;
        runtime::Worker synchronousWorker{ testRunStartedId, broadcaster, idGenerator, options, supportCodeLibrary, programContext };

        failing |= IsFailing(util::GetWorstTestStepResult(synchronousWorker.RunBeforeAllHooks()).status, options.dryRun);

        if (!failing)
        {
            ParallelBroadcaster parallelBroadcaster{ broadcaster, threadPool };
            tasks.emplace_back(parallelBroadcaster.BroadcastEventTask());

            runtime::Worker parallelWorker{ testRunStartedId, parallelBroadcaster, idGenerator, options, supportCodeLibrary, programContext };

            auto assembledTestSuites = assemble::AssembleTestSuites(supportCodeLibrary, testRunStartedId, broadcaster, sourcedPickles, idGenerator);

            coro::latch taskLatch{ 10 * std::ranges::distance(assembledTestSuites | std::views::transform([](const auto& suite) -> std::list<AssembledTestCase>&
                                                                                        {
                                                                                            return suite.testCases;
                                                                                        }) |
                                                              std::views::join) };

            for (const auto& assembledTestSuite : assembledTestSuites)
                for (const auto& assembledTestCase : assembledTestSuite.testCases)
                    for (auto i{ 0 }; i < 10; ++i)
                        tasks.emplace_back(RunTestCase(threadPool, taskLatch, parallelWorker, assembledTestSuite.gherkinDocument, assembledTestCase, programContext, failing));

            tasks.emplace_back(parallelBroadcaster.ShutdownTask(taskLatch));

            coro::sync_wait(coro::when_all(std::move(tasks)));
        }

        failing |= IsFailing(util::GetWorstTestStepResult(synchronousWorker.RunAfterAllHooks()).status, options.dryRun);

        return !failing;
    }
}
