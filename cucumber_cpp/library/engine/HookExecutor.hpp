#ifndef ENGINE_HOOKEXECUTOR_HPP
#define ENGINE_HOOKEXECUTOR_HPP

#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include <functional>
#include <set>
#include <string>

namespace cucumber_cpp::library::engine
{

    struct HookExecutor
    {
        virtual void BeforeAll() = 0;
        virtual void AfterAll() = 0;

        virtual void BeforeFeature() = 0;
        virtual void AfterFeature() = 0;

        virtual void BeforeScenario() = 0;
        virtual void AfterScenario() = 0;

        virtual void BeforeStep() = 0;
        virtual void AfterStep() = 0;
    };

    struct HookExecutorImpl : HookExecutor
    {
        explicit HookExecutorImpl(::cucumber_cpp::engine::ContextManager& contextManager);

        void BeforeAll() override;
        void AfterAll() override;

        void BeforeFeature() override;
        void AfterFeature() override;

        void BeforeScenario() override;
        void AfterScenario() override;

        void BeforeStep() override;
        void AfterStep() override;

    private:
        void ExecuteHook(Context& context, HookType hook, const std::set<std::string, std::less<>>& tags);

        ::cucumber_cpp::engine::ContextManager& contextManager;
    };
}

#endif
