#ifndef RUNTIME_TEST_CASE_RUNNER_HPP
#define RUNTIME_TEST_CASE_RUNNER_HPP

#include "cucumber/gherkin/app.hpp"
#include "cucumber/gherkin/exceptions.hpp"
#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/gherkin/pickle_compiler.hpp"
#include "cucumber/gherkin/utils.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/gherkin_document.hpp"
#include "cucumber/messages/parameter_type.hpp"
#include "cucumber/messages/parse_error.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include "cucumber/messages/source.hpp"
#include "cucumber/messages/source_reference.hpp"
#include "cucumber/messages/step_definition.hpp"
#include "cucumber/messages/step_definition_pattern.hpp"
#include "cucumber/messages/step_match_argument.hpp"
#include "cucumber/messages/suggestion.hpp"
#include "cucumber/messages/test_case.hpp"
#include "cucumber/messages/test_case_finished.hpp"
#include "cucumber/messages/test_case_started.hpp"
#include "cucumber/messages/test_run_finished.hpp"
#include "cucumber/messages/test_run_hook_finished.hpp"
#include "cucumber/messages/test_run_hook_started.hpp"
#include "cucumber/messages/test_run_started.hpp"
#include "cucumber/messages/test_step.hpp"
#include "cucumber/messages/test_step_finished.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber/messages/timestamp.hpp"
#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/Body.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/assemble/AssembleTestSuites.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestCase.hpp"
#include "cucumber_cpp/library/assemble/AssembledTestSuite.hpp"
#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Timestamp.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/tag_expression/Parser.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <algorithm>
#include <any>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
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
            support::SupportCodeLibrary supportCodeLibrary,
            Context& testSuiteContext);

        cucumber::messages::test_step_result_status Run();

        bool RunAttempt(std::size_t attempt, bool moreAttemptsAvailable);

        cucumber::messages::test_step_result RunHook(const HookRegistry::Definition& hookDefinition, bool isBeforeHook, Context& testCaseContext);

        std::vector<cucumber::messages::test_step_result> RunStepHooks(const cucumber::messages::pickle_step& pickleStep, HookType hookType, Context& testCaseContext);

        cucumber::messages::test_step_result RunStep(const cucumber::messages::pickle_step& pickleStep, const cucumber::messages::test_step& testStep, Context& testCaseContext);

        cucumber::messages::test_step_result InvokeStep(std::unique_ptr<Body> body, const ExecuteArgs& args = {});
        cucumber::messages::test_step_result GetWorstStepResult() const;

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
        support::SupportCodeLibrary supportCodeLibrary;
        Context& testSuiteContext;

        std::vector<cucumber::messages::test_step_result> testStepResults;
    };
}

#endif
