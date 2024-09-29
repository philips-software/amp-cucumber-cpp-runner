#ifndef ENGINE_CONTEXTMANAGER_HPP
#define ENGINE_CONTEXTMANAGER_HPP

#include "cucumber_cpp/Context.hpp"
#include "cucumber_cpp/TraceTime.hpp"
#include "cucumber_cpp/engine/Result.hpp"
#include "cucumber_cpp/engine/RuleInfo.hpp"
#include "cucumber_cpp/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/engine/StepInfo.hpp"
#include <memory>
#include <stack>

namespace cucumber_cpp::engine
{
    struct CurrentContext : Context
    {
        explicit CurrentContext(std::shared_ptr<ContextStorageFactory> contextStorageFactory);
        explicit CurrentContext(CurrentContext* parent);

        [[nodiscard]] Result ExecutionStatus() const;

        void Start();
        [[nodiscard]] TraceTime::Duration Duration() const;

    protected:
        void ExecutionStatus(Result result);

    private:
        CurrentContext* parent{ nullptr };

        Result executionStatus{ Result::passed };
        TraceTime traceTime;
    };

    struct RunnerContext : CurrentContext
    {
        using CurrentContext::CurrentContext;
        using CurrentContext::ExecutionStatus;
    };

    struct ProgramContext : RunnerContext
    {
        explicit ProgramContext(std::shared_ptr<ContextStorageFactory> contextStorageFactory);
    };

    template<class T>
    struct NestedContext : RunnerContext
    {
        NestedContext(RunnerContext& parent, const T& info)
            : RunnerContext{ &parent }
            , info{ info }
        {}

        const T& info;
    };

    using FeatureContext = NestedContext<FeatureInfo>;
    using RuleContext = NestedContext<RuleInfo>;
    using ScenarioContext = NestedContext<ScenarioInfo>;
    using StepContext = NestedContext<StepInfo>;

    struct ContextManager
    {
        explicit ContextManager(std::shared_ptr<ContextStorageFactory> contextStorageFactory);

        struct ScopedContextLock
        {
            explicit ScopedContextLock(std::stack<std::unique_ptr<RunnerContext>>& activeContextStack)
                : activeContextStack{ activeContextStack }
            {}

            ScopedContextLock(const ScopedContextLock&) = delete;
            ScopedContextLock& operator=(const ScopedContextLock&) = delete;
            ScopedContextLock(ScopedContextLock&&) = delete;
            ScopedContextLock& operator=(ScopedContextLock&&) = delete;

            std::stack<std::unique_ptr<RunnerContext>>& activeContextStack;
        };

        template<class T>
        [[nodiscard]] auto& StartScope(const T& info)
        {
            activeContextStack.push(std::make_unique<struct NestedContext<T>>(*activeContextStack.top(), info));

            return activeContextStack;
        }

        [[nodiscard]] auto& StartScope(const StepInfo& stepInfo)
        {
            stepContext.push(std::make_unique<NestedContext<StepInfo>>(CurrentContext(), stepInfo));

            return activeContextStack;
        }

        RunnerContext& CurrentContext();
        [[nodiscard]] const RunnerContext& CurrentContext() const;

        template<class T>
        NestedContext<T>& CurrentContextAs()
        {
            return static_cast<NestedContext<T>&>(*activeContextStack.top());
        }

        RunnerContext& StepContext();
        [[nodiscard]] const RunnerContext& StepContext() const;

    private:
        std::stack<std::unique_ptr<RunnerContext>> activeContextStack;
        std::stack<std::unique_ptr<RunnerContext>> stepContext;
    };
}

#endif
