#include "cucumber_cpp/library/engine/Step.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/runtime/NestedTestCaseRunner.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/DocString.hpp"
#include "cucumber_cpp/library/util/Table.hpp"
#include "cucumber_cpp/library/util/TransformDocString.hpp"
#include "cucumber_cpp/library/util/TransformTable.hpp"
#include <optional>
#include <string>
#include <utility>

namespace cucumber_cpp::library::engine
{
    StepBase::StepBase(const runtime::NestedTestCaseRunner& nestedTestCaseRunner, util::Broadcaster& broadCaster, Context& context, engine::StepOrHookStarted stepOrHookStarted, const std::optional<util::Table>& dataTable, const std::optional<util::DocString>& docString)
        : ExecutionContext{ broadCaster, context, std::move(stepOrHookStarted) }
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
        nestedTestCaseRunner.Step(step, util::TransformDocString(docString));
    }

    void StepBase::Step(const std::string& step, const std::optional<util::Table>& dataTable) const
    {
        nestedTestCaseRunner.Step(step, util::TransformTable(dataTable));
    }

    void StepBase::Step(const std::string& step, const std::optional<util::Table>& dataTable, const std::optional<util::DocString>& docString) const
    {
        nestedTestCaseRunner.Step(step, util::TransformTable(dataTable), util::TransformDocString(docString));
    }
}
