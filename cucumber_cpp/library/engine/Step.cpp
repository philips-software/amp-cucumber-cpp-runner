#include "cucumber_cpp/library/engine/Step.hpp"
#include "cucumber/messages/pickle_doc_string.hpp"
#include "cucumber/messages/pickle_table.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/runtime/NestedTestCaseRunner.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <optional>
#include <string>

namespace cucumber_cpp::library::engine
{
    StepBase::StepBase(const runtime::NestedTestCaseRunner& nestedTestCaseRunner, util::Broadcaster& broadCaster, Context& context, engine::StepOrHookStarted stepOrHookStarted, const std::optional<util::Table>& dataTable, const std::optional<util::DocString>& docString)
        : ExecutionContext{ broadCaster, context, stepOrHookStarted }
        , nestedTestCaseRunner{ nestedTestCaseRunner }
        , dataTable{ dataTable }
        , docString{ docString }
    {}

    void StepBase::Step(const std::string& step) const
    {
        nestedTestCaseRunner.Step(step);
    }

    void StepBase::Step(const std::string& step, const std::optional<util::DocString>& docString) const
    {
        // nestedTestCaseRunner.Step(step, docString);
    }

    void StepBase::Step(const std::string& step, const std::optional<util::Table>& dataTable) const
    {
        // nestedTestCaseRunner.Step(step, dataTable);
    }

    void StepBase::Step(const std::string& step, const std::optional<util::Table>& dataTable, const std::optional<util::DocString>& docString) const
    {
        // nestedTestCaseRunner.Step(step, dataTable, docString);
    }
}
