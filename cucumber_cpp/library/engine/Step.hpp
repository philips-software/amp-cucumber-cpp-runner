#ifndef ENGINE_STEP_HPP
#define ENGINE_STEP_HPP

// IWYU pragma: private, include "cucumber_cpp/Steps.hpp"
// IWYU pragma: friend cucumber_cpp/.*

#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/util/DocString.hpp"
#include "cucumber_cpp/library/util/Table.hpp"
#include <optional>
#include <string>

namespace cucumber_cpp::library::runtime
{
    struct NestedTestCaseRunner;
}

namespace cucumber_cpp::library::util
{
    struct Broadcaster;
}

namespace cucumber_cpp::library::engine
{
    struct StepBase : ExecutionContext
    {
        StepBase(const runtime::NestedTestCaseRunner& nestedTestCaseRunner, util::Broadcaster& broadCaster, Context& context, engine::StepOrHookStarted stepOrHookStarted, const std::optional<util::Table>& dataTable, const std::optional<util::DocString>& docString);
        virtual ~StepBase() = default;

        virtual void SetUp()
        {
            /* nothing to do */
        }

        virtual void TearDown()
        {
            /* nothing to do */
        }

    protected:
        void Step(const std::string& step) const;
        void Step(const std::string& step, const std::optional<util::DocString>& docString) const;
        void Step(const std::string& step, const std::optional<util::Table>& dataTable) const;
        void Step(const std::string& step, const std::optional<util::Table>& dataTable, const std::optional<util::DocString>& docString) const;

        const runtime::NestedTestCaseRunner& nestedTestCaseRunner;

        const std::optional<util::Table>& dataTable;
        const std::optional<util::DocString>& docString;
    };
}

#endif
