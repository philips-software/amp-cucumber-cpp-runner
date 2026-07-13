#ifndef ENGINE_HOOK_HPP
#define ENGINE_HOOK_HPP

#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/util/ScenarioInfo.hpp"
#include "cucumber_cpp/library/util/StepOrHookStarted.hpp"
#include <optional>

namespace cucumber_cpp::library::util
{
    struct Broadcaster;
}

namespace cucumber_cpp::library::engine
{
    struct HookBase : engine::ExecutionContext
    {
        HookBase(util::Broadcaster& broadCaster, Context& context, util::StepOrHookStarted stepOrHookStarted, bool hasError);

        virtual ~HookBase() = default;

        virtual void SetUp()
        {
            /* nothing to do */
        }

        virtual void TearDown()
        {
            /* nothing to do */
        }

    protected:
        const bool hasError;
    };

    struct GlobalHookImpl : HookBase
    {
        GlobalHookImpl(util::Broadcaster& broadCaster, Context& context, util::StepOrHookStarted stepOrHookStarted, const std::optional<util::ScenarioInfo>& scenarioInfo, bool hasError);
    };

    struct HookImpl : HookBase // NOSONAR(cpp:S3656) -
    {
        HookImpl(util::Broadcaster& broadCaster, Context& context, util::StepOrHookStarted stepOrHookStarted, std::optional<util::ScenarioInfo> scenarioInfo, bool hasError);

    protected:
        [[nodiscard]] const util::ScenarioInfo& ScenarioInfo() const;

    private:
        util::ScenarioInfo scenarioInfo;
    };
}

#endif
