#include "cucumber_cpp/library/engine/Hook.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/StepOrHookStarted.hpp"
#include <utility>

namespace cucumber_cpp::library::engine
{
    HookBase::HookBase(util::Broadcaster& broadCaster, Context& context, util::StepOrHookStarted stepOrHookStarted, bool hasError)
        : engine::ExecutionContext{ broadCaster, context, std::move(stepOrHookStarted) }
        , hasError{ hasError }
    {}
}
