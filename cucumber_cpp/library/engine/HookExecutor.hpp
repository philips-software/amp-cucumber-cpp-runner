#ifndef ENGINE_HOOKEXECUTOR_HPP
#define ENGINE_HOOKEXECUTOR_HPP

#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/util/Immoveable.hpp"
#include <functional>
#include <optional>
#include <set>
#include <string>

namespace cucumber_cpp::library::engine
{

    struct HookExecutor
    {
    protected:
        ~HookExecutor() = default;

    public:
        struct ProgramScope;
        struct FeatureScope;
        struct ScenarioScope;
        struct StepScope;

        [[nodiscard]] virtual ProgramScope BeforeAll() = 0;
        [[nodiscard]] virtual std::optional<FeatureScope> FeatureStart() = 0;
        [[nodiscard]] virtual std::optional<ScenarioScope> ScenarioStart() = 0;
        [[nodiscard]] virtual StepScope StepStart() = 0;

    private:
        struct ScopedHook;
    };

    struct HookPair
    {
        const HookType before;
        const HookType after;
    };

    struct HookExecutor::ScopedHook : util::Immoveable
    {
        ScopedHook(cucumber_cpp::library::engine::RunnerContext& runnerContext, HookPair hookPair, const std::set<std::string, std::less<>>& tags);
        ~ScopedHook();

    private:
        cucumber_cpp::library::engine::RunnerContext& runnerContext;
        HookPair hookPair;
        const std::set<std::string, std::less<>>& tags;
        bool executeHooks{ true };
    };

    struct HookExecutor::ProgramScope : private ScopedHook
    {
        explicit ProgramScope(cucumber_cpp::library::engine::ContextManager& contextManager);
    };

    struct HookExecutor::FeatureScope : private ScopedHook
    {
        explicit FeatureScope(cucumber_cpp::library::engine::ContextManager& contextManager);
    };

    struct HookExecutor::ScenarioScope : private ScopedHook
    {
        explicit ScenarioScope(cucumber_cpp::library::engine::ContextManager& contextManager);
    };

    struct HookExecutor::StepScope : private ScopedHook
    {
        explicit StepScope(cucumber_cpp::library::engine::ContextManager& contextManager);
    };

    struct HookExecutorImpl : HookExecutor
    {
        explicit HookExecutorImpl(cucumber_cpp::library::engine::ContextManager& contextManager);
        virtual ~HookExecutorImpl() = default;

        [[nodiscard]] ProgramScope
        BeforeAll() override;
        [[nodiscard]] std::optional<FeatureScope> FeatureStart() override;
        [[nodiscard]] std::optional<ScenarioScope> ScenarioStart() override;
        [[nodiscard]] StepScope StepStart() override;

    private:
        cucumber_cpp::library::engine::ContextManager& contextManager;
    };
}

#endif
