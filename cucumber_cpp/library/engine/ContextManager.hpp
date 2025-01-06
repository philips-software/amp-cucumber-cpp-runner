#ifndef ENGINE_CONTEXTMANAGER_HPP
#define ENGINE_CONTEXTMANAGER_HPP

#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/TraceTime.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/util/Immoveable.hpp"
#include <memory>
#include <stack>
#include <stdexcept>

namespace cucumber_cpp::library::engine
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
    using NestedStepContext = NestedContext<NestedStepInfo>;

    struct ContextNotAvailable : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    struct ContextManager
    {
        explicit ContextManager(std::shared_ptr<ContextStorageFactory> contextStorageFactory);

        cucumber_cpp::library::engine::ProgramContext& ProgramContext();
        cucumber_cpp::library::engine::ProgramContext& ProgramContext() const;

        struct ScopedFeautureContext;
        struct ScopedRuleContext;
        struct ScopedScenarioContext;
        struct ScopedStepContext;

    public:
        [[nodiscard]] ScopedFeautureContext CreateFeatureContext(const FeatureInfo& featureInfo);
        cucumber_cpp::library::engine::FeatureContext& FeatureContext();

        [[nodiscard]] ScopedRuleContext CreateRuleContext(const RuleInfo& ruleInfo);
        cucumber_cpp::library::engine::RuleContext& RuleContext();

        [[nodiscard]] ScopedScenarioContext CreateScenarioContext(const ScenarioInfo& scenarioInfo);
        cucumber_cpp::library::engine::ScenarioContext& ScenarioContext();

        [[nodiscard]] ScopedStepContext CreateStepContext(const StepInfo& stepInfo);

        cucumber_cpp::library::engine::StepContext& StepContext();

        cucumber_cpp::library::engine::RunnerContext& CurrentContext();

    private:
        void DisposeFeatureContext();
        void DisposeRuleContext();
        void DisposeScenarioContext();
        void DisposeStepContext();

        std::shared_ptr<cucumber_cpp::library::engine::ProgramContext> programContext;
        std::shared_ptr<cucumber_cpp::library::engine::FeatureContext> featureContext;
        std::shared_ptr<cucumber_cpp::library::engine::RuleContext> ruleContext;
        std::shared_ptr<cucumber_cpp::library::engine::ScenarioContext> scenarioContext;

        std::stack<std::shared_ptr<cucumber_cpp::library::engine::RunnerContext>> runnerContext;
        std::stack<std::shared_ptr<cucumber_cpp::library::engine::StepContext>> stepContext;
    };

    struct ContextManager::ScopedFeautureContext : library::util::Immoveable
    {
        ScopedFeautureContext(ContextManager& contextManager);
        ~ScopedFeautureContext();

    private:
        ContextManager& contextManager;
    };

    struct ContextManager::ScopedRuleContext : library::util::Immoveable
    {
        ScopedRuleContext(ContextManager& contextManager);
        ~ScopedRuleContext();

    private:
        ContextManager& contextManager;
    };

    struct ContextManager::ScopedScenarioContext : library::util::Immoveable
    {
        ScopedScenarioContext(ContextManager& contextManager);
        ~ScopedScenarioContext();

    private:
        ContextManager& contextManager;
    };

    struct ContextManager::ScopedStepContext : library::util::Immoveable
    {
        ScopedStepContext(ContextManager& contextManager);
        ~ScopedStepContext();

    private:
        ContextManager& contextManager;
    };
}

#endif
