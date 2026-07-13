#include "cucumber_cpp/library/engine/Hook.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/ScenarioInfo.hpp"
#include "cucumber_cpp/library/util/StepOrHookStarted.hpp"
#include <optional>
#include <utility>

namespace cucumber_cpp::library::engine
{
    HookBase::HookBase(util::Broadcaster& broadCaster, Context& context, util::StepOrHookStarted stepOrHookStarted, bool hasError)
        : engine::ExecutionContext{ broadCaster, context, std::move(stepOrHookStarted) }
        , hasError{ hasError }
    {}

    GlobalHookImpl::GlobalHookImpl(util::Broadcaster& broadCaster, Context& context, util::StepOrHookStarted stepOrHookStarted, [[maybe_unused]] const std::optional<util::ScenarioInfo>& scenarioInfo, bool hasError)
        : HookBase{ broadCaster, context, std::move(stepOrHookStarted), hasError }
    {}

    HookImpl::HookImpl(util::Broadcaster& broadCaster, Context& context, util::StepOrHookStarted stepOrHookStarted, std::optional<util::ScenarioInfo> scenarioInfo, bool hasError)
        : HookBase{ broadCaster, context, std::move(stepOrHookStarted), hasError }
        , scenarioInfo{ std::move(scenarioInfo.value()) }
    {}

    const util::ScenarioInfo& HookImpl::ScenarioInfo() const
    {
        return scenarioInfo;
    }
}
