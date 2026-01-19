#ifndef RUNTIME_NESTED_TEST_CASE_RUNNER_HPP
#define RUNTIME_NESTED_TEST_CASE_RUNNER_HPP

#include "cucumber/messages/pickle_doc_string.hpp"
#include "cucumber/messages/pickle_table.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <cstddef>
#include <optional>
#include <string>

namespace cucumber_cpp::library::runtime
{
    struct NestedTestCaseRunnerError
    {
        std::size_t nesting;
        cucumber::messages::test_step_result status;
        std::string text;
    };

    struct NestedTestCaseRunner
    {
        NestedTestCaseRunner(std::size_t nesting, const support::SupportCodeLibrary& supportCodeLibrary, util::Broadcaster& broadcaster, Context& testCaseContext, const cucumber::messages::test_step_started& testStepStarted);

        void Step(const std::string& step) const;
        void Step(const std::string& step, const std::optional<cucumber::messages::pickle_doc_string>& docString) const;
        void Step(const std::string& step, const std::optional<cucumber::messages::pickle_table>& dataTable) const;
        void Step(const std::string& step, const std::optional<cucumber::messages::pickle_table>& dataTable, const std::optional<cucumber::messages::pickle_doc_string>& docString) const;

    private:
        std::size_t nesting;
        const support::SupportCodeLibrary& supportCodeLibrary;
        util::Broadcaster& broadcaster;
        Context& testCaseContext;
        const cucumber::messages::test_step_started& testStepStarted;
    };
}

#endif
