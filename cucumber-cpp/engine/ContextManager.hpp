#ifndef ENGINE_CONTEXTMANAGER_HPP
#define ENGINE_CONTEXTMANAGER_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "cucumber-cpp/engine/Result.hpp"
#include "cucumber-cpp/engine/RuleInfo.hpp"
#include "cucumber-cpp/engine/ScenarioInfo.hpp"
#include "cucumber-cpp/engine/StepInfo.hpp"
#include <memory>
#include <optional>
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

    struct FeatureContext : RunnerContext
    {
        FeatureContext(RunnerContext& parent, const FeatureInfo& featureInfo);

        const FeatureInfo& featureInfo;
    };

    struct RuleContext : RunnerContext
    {
        RuleContext(RunnerContext& parent, const RuleInfo& ruleInfo);

        const RuleInfo& ruleInfo;
    };

    struct ScenarioContext : RunnerContext
    {
        ScenarioContext(RunnerContext& parent, const ScenarioInfo& scenarioInfo);

        const ScenarioInfo& scenarioInfo;
    };

    struct StepContext : RunnerContext
    {
        StepContext(RunnerContext& parent, const StepInfo& stepInfo);

        const StepInfo& stepInfo;
    };

    struct ContextManager
    {
        explicit ContextManager(std::shared_ptr<ContextStorageFactory> contextStorageFactory);

        void StartFeature(const FeatureInfo& featureInfo);
        void StopFeature();

        void StartRule(const RuleInfo& ruleInfo);
        void StopRule();

        void StartScenario(const ScenarioInfo& scenarioInfo);
        void StopScenario();

        void StartStep(const StepInfo& stepInfo);
        void StopStep();

        struct ProgramContext& ProgramContext();
        [[nodiscard]] const struct ProgramContext& ProgramContext() const;

        struct FeatureContext& FeatureContext();
        struct RuleContext& RuleContext();
        struct ScenarioContext& ScenarioContext();
        struct StepContext& StepContext();

        RunnerContext& CurrentContext();

    private:
        struct ProgramContext programContext;
        std::optional<struct FeatureContext> featureContext;
        std::optional<struct RuleContext> ruleContext;
        std::optional<struct ScenarioContext> scenarioContext;
        std::stack<struct StepContext> stepContext;
    };
}

#endif
