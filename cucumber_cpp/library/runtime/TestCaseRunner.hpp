#ifndef RUNTIME_TEST_CASE_RUNNER_HPP
#define RUNTIME_TEST_CASE_RUNNER_HPP

#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber/messages/test_case.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/support/HookRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/HookData.hpp"
#include <cstddef>
#include <vector>

namespace cucumber_cpp::library::runtime
{
    struct TestCaseRunner
    {
        TestCaseRunner(util::Broadcaster& broadcaster,
            cucumber::gherkin::id_generator_ptr idGenerator,
            const cucumber::messages::gherkin_document& gherkinDocument,
            const cucumber::messages::pickle& pickle,
            const cucumber::messages::test_case& testCase,
            std::size_t retries,
            bool skip,
            support::SupportCodeLibrary& supportCodeLibrary,
            Context& testSuiteContext);

        cucumber::messages::test_step_result_status Run();

        bool RunAttempt(std::size_t attempt, bool moreAttemptsAvailable);

        cucumber::messages::test_step_result RunHook(const support::HookRegistry::Definition& hookDefinition, bool isBeforeHook, Context& testCaseContext, const cucumber::messages::test_step_started& testStepStarted, bool hasError);

        std::vector<cucumber::messages::test_step_result> RunStepHooks(const cucumber::messages::pickle_step& pickleStep, util::HookType hookType, Context& testCaseContext, const cucumber::messages::test_step_started& testStepStarted);

        cucumber::messages::test_step_result RunStep(const cucumber::messages::pickle_step& pickleStep, const cucumber::messages::test_step& testStep, Context& testCaseContext, const cucumber::messages::test_step_started& testStepStarted);

        [[nodiscard]] cucumber::messages::test_step_result GetWorstStepResult() const;

        bool ShouldSkipHook(bool isBeforeHook);
        bool IsSkippingSteps();

    private:
        util::Broadcaster& broadcaster;
        cucumber::gherkin::id_generator_ptr idGenerator;
        const cucumber::messages::gherkin_document& gherkinDocument;
        const cucumber::messages::pickle& pickle;
        const cucumber::messages::test_case& testCase;
        std::size_t maximumAttempts;
        bool skip;
        support::SupportCodeLibrary& supportCodeLibrary;
        Context& testSuiteContext;

        std::vector<cucumber::messages::test_step_result> testStepResults;
    };
}

#endif
