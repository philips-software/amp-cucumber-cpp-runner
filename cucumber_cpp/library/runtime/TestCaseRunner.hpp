#ifndef RUNTIME_TEST_CASE_RUNNER_HPP
#define RUNTIME_TEST_CASE_RUNNER_HPP

#include "cucumber/gherkin/IdGenerator.hpp"
#include "cucumber/messages/GherkinDocument.hpp"
#include "cucumber/messages/Pickle.hpp"
#include "cucumber/messages/PickleStep.hpp"
#include "cucumber/messages/TestCase.hpp"
#include "cucumber/messages/TestStep.hpp"
#include "cucumber/messages/TestStepResult.hpp"
#include "cucumber/messages/TestStepResultStatus.hpp"
#include "cucumber/messages/TestStepStarted.hpp"
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
            cucumber::gherkin::IdGeneratorPtr idGenerator,
            const cucumber::messages::GherkinDocument& gherkinDocument,
            const cucumber::messages::Pickle& pickle,
            const cucumber::messages::TestCase& testCase,
            std::size_t retries,
            bool skip,
            support::SupportCodeLibrary& supportCodeLibrary,
            Context& testSuiteContext);

        cucumber::messages::TestStepResultStatus Run();

        bool RunAttempt(std::size_t attempt, bool moreAttemptsAvailable);

        cucumber::messages::TestStepResult RunHook(const support::HookRegistry::Definition& hookDefinition, bool isBeforeHook, Context& testCaseContext, const cucumber::messages::TestStepStarted& testStepStarted, bool hasError);

        std::vector<cucumber::messages::TestStepResult> RunStepHooks(const cucumber::messages::PickleStep& pickleStep, util::HookType hookType, Context& testCaseContext, const cucumber::messages::TestStepStarted& testStepStarted);

        cucumber::messages::TestStepResult RunStep(const cucumber::messages::PickleStep& pickleStep, const cucumber::messages::TestStep& testStep, Context& testCaseContext, const cucumber::messages::TestStepStarted& testStepStarted);

        [[nodiscard]] cucumber::messages::TestStepResult GetWorstStepResult() const;

        bool ShouldSkipHook(bool isBeforeHook);
        bool IsSkippingSteps();

    private:
        util::Broadcaster& broadcaster;
        cucumber::gherkin::IdGeneratorPtr idGenerator;
        const cucumber::messages::GherkinDocument& gherkinDocument;
        const cucumber::messages::Pickle& pickle;
        const cucumber::messages::TestCase& testCase;
        std::size_t maximumAttempts;
        bool skip;
        support::SupportCodeLibrary& supportCodeLibrary;
        Context& testSuiteContext;

        std::vector<cucumber::messages::TestStepResult> testStepResults;
    };
}

#endif
