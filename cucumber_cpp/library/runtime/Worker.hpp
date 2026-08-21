#ifndef RUNTIME_WORKER_HPP
#define RUNTIME_WORKER_HPP

#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/messages/Feature.hpp"
#include "cucumber/messages/GherkinDocument.hpp"
#include "cucumber/messages/TestStepResult.hpp"
#include "cucumber/messages/TestStepResultStatus.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestCase.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <cucumber/messages/Pickle.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::runtime
{
    struct GlobalHookError : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    struct Worker
    {
        Worker(std::string_view testRunStartedId,
            util::Broadcaster& broadcaster,
            cucumber::gherkin::IdGeneratorPtr idGenerator,
            const support::RunOptions::Runtime& options,
            support::SupportCodeLibrary& supportCodeLibrary,
            Context& programContext);

        std::vector<cucumber::messages::TestStepResult> RunBeforeAllHooks();
        std::vector<cucumber::messages::TestStepResult> RunAfterAllHooks();

        bool RunTestSuite(const assemble::AssembledTestSuite& assembledTestSuite, bool failing);
        bool RunTestCase(const cucumber::messages::GherkinDocument& gherkinDocument, const assemble::AssembledTestCase& assembledTestCase, Context& testSuiteContext, bool failing);

    private:
        std::vector<cucumber::messages::TestStepResult> RunBeforeTestSuiteHooks(const cucumber::messages::Feature& feature, Context& context);
        std::vector<cucumber::messages::TestStepResult> RunAfterTestSuiteHooks(const cucumber::messages::Feature& feature, Context& context);

        cucumber::messages::TestStepResult RunTestHook(const std::string& id, Context& context);

        bool IsStatusFailed(cucumber::messages::TestStepResultStatus status) const;

        std::string_view testRunStartedId;
        util::Broadcaster& broadcaster;
        cucumber::gherkin::IdGeneratorPtr idGenerator;
        const support::RunOptions::Runtime& options;
        support::SupportCodeLibrary& supportCodeLibrary;
        Context& programContext;
    };
}

#endif
