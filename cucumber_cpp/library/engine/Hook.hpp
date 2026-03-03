#ifndef ENGINE_HOOK_HPP
#define ENGINE_HOOK_HPP

#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/util/StepOrHookStarted.hpp"

namespace cucumber_cpp::library::util
{
    struct Broadcaster;
}

namespace cucumber_cpp::library::engine
{
    struct HookBase : engine::ExecutionContext
    {
        HookBase(util::Broadcaster& broadCaster, Context& context, util::StepOrHookStarted stepOrHookStarted);

        virtual ~HookBase() = default;

        virtual void SetUp()
        {
            /* nothing to do */
        }

        virtual void TearDown()
        {
            /* nothing to do */
        }
    };
}

#endif
