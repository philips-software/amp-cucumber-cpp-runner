#ifndef UTIL_HOOK_FACTORY_HPP
#define UTIL_HOOK_FACTORY_HPP

#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/StepOrHookStarted.hpp"
#include <memory>

namespace cucumber_cpp::library::util
{
    using HookFactory = std::unique_ptr<Body> (&)(Broadcaster& broadCaster, Context& context, StepOrHookStarted stepOrHookStarted);

    template<class T>
    std::unique_ptr<Body> HookBodyFactory(Broadcaster& broadCaster, Context& context, StepOrHookStarted stepOrHookStarted)
    {
        return std::make_unique<T>(broadCaster, context, stepOrHookStarted);
    }
}

#endif
