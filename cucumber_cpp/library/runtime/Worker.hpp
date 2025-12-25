#ifndef RUNTIME_WORKER_HPP
#define RUNTIME_WORKER_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestCase.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::runtime
{
    struct Worker
    {
        Worker(std::string_view testRunStartedId,
            util::Broadcaster& broadcaster,
            cucumber::gherkin::id_generator_ptr idGenerator,
            const support::RunOptions::Runtime& options,
            support::SupportCodeLibrary& supportCodeLibrary,
            Context& programContext);

        std::vector<cucumber::messages::test_step_result> RunBeforeAllHooks();
        std::vector<cucumber::messages::test_step_result> RunAfterAllHooks();

        bool RunTestSuite(const assemble::AssembledTestSuite& assembledTestSuite, bool failing);
        bool RunTestCase(const cucumber::messages::gherkin_document& gherkinDocument, const assemble::AssembledTestCase& assembledTestCase, Context& testSuiteContext, bool failing);

    private:
        std::vector<cucumber::messages::test_step_result> RunBeforeTestSuiteHooks(const cucumber::messages::feature& feature, Context& context);
        std::vector<cucumber::messages::test_step_result> RunAfterTestSuiteHooks(const cucumber::messages::feature& feature, Context& context);

        cucumber::messages::test_step_result RunTestHook(std::string id, Context& context);

        bool IsStatusFailed(cucumber::messages::test_step_result_status status);

        std::string_view testRunStartedId;
        util::Broadcaster& broadcaster;
        cucumber::gherkin::id_generator_ptr idGenerator;
        const support::RunOptions::Runtime& options;
        support::SupportCodeLibrary& supportCodeLibrary;
        Context& programContext;
    };
}

#endif
