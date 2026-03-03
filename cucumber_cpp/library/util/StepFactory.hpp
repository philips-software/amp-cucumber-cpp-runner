#ifndef UTIL_STEP_FACTORY_HPP
#define UTIL_STEP_FACTORY_HPP

#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/DocString.hpp"
#include "cucumber_cpp/library/util/StepOrHookStarted.hpp"
#include "cucumber_cpp/library/util/Table.hpp"
#include <memory>
#include <optional>

namespace cucumber_cpp::library::runtime
{
    struct NestedTestCaseRunner;
}

namespace cucumber_cpp::library::util
{
    using StepFactory = std::unique_ptr<Body> (&)(const runtime::NestedTestCaseRunner&, Broadcaster& broadCaster, Context&, StepOrHookStarted stepOrHookStarted, const std::optional<Table>&, const std::optional<DocString>&);

    template<class T>
    std::unique_ptr<Body> StepBodyFactory(const runtime::NestedTestCaseRunner& nestedTestCaseRunner, Broadcaster& broadCaster, Context& context, StepOrHookStarted stepOrHookStarted, const std::optional<Table>& dataTable, const std::optional<DocString>& docString)
    {
        return std::make_unique<T>(nestedTestCaseRunner, broadCaster, context, stepOrHookStarted, dataTable, docString);
    }
}
#endif
