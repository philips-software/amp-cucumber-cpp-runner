#ifndef UTIL_HOOK_FACTORY_HPP
#define UTIL_HOOK_FACTORY_HPP

#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/ScenarioInfo.hpp"
#include "cucumber_cpp/library/util/StepOrHookStarted.hpp"
#include "cucumber_cpp/library/util/TestStepResult.hpp"
#include <memory>
#include <optional>
#include <utility>

namespace cucumber_cpp::library::util
{
    using HookFactory = std::unique_ptr<Body> (&)(util::TestStepResult& testStepResult, Broadcaster& broadCaster, Context& context, StepOrHookStarted stepOrHookStarted, std::optional<util::ScenarioInfo> scenarioInfo, bool hasError);

    template<class T>
    std::unique_ptr<Body> HookBodyFactory(util::TestStepResult& testStepResult, Broadcaster& broadCaster, Context& context, StepOrHookStarted stepOrHookStarted, std::optional<util::ScenarioInfo> scenarioInfo, bool hasError)
    {
        return std::make_unique<T>(testStepResult, broadCaster, context, std::move(stepOrHookStarted), std::move(scenarioInfo), hasError);
    }
}

#endif
