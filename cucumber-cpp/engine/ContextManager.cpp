#include "cucumber-cpp/engine/ContextManager.hpp"

namespace cucumber_cpp::engine
{
    CurrentContext::CurrentContext(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
        : Context{ contextStorageFactory }
    {}

    CurrentContext::CurrentContext(CurrentContext* parent)
        : Context{ parent }
        , parent{ parent }

    {
        Start();
    }

    [[nodiscard]] Result CurrentContext::ExecutionStatus() const
    {
        return executionStatus;
    }

    void CurrentContext::Start()
    {
        traceTime.Start();
    }

    TraceTime::Duration CurrentContext::Duration() const
    {
        return traceTime.Delta();
    }

    void CurrentContext::ExecutionStatus(Result result)
    {
        if (result > executionStatus)
            executionStatus = result;

        if (parent != nullptr)
            parent->ExecutionStatus(result);
    }

    ProgramContext::ProgramContext(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
        : RunnerContext{ std::move(contextStorageFactory) }
    {
    }

    FeatureContext::FeatureContext(RunnerContext& parent, const FeatureInfo& featureInfo)
        : RunnerContext{ &parent }
        , featureInfo{ featureInfo }
    {
    }

    RuleContext::RuleContext(RunnerContext& parent, const RuleInfo& ruleInfo)
        : RunnerContext{ &parent }
        , ruleInfo{ ruleInfo }
    {
    }

    ScenarioContext::ScenarioContext(RunnerContext& parent, const ScenarioInfo& scenarioInfo)
        : RunnerContext{ &parent }
        , scenarioInfo{ scenarioInfo }
    {
    }

    StepContext::StepContext(RunnerContext& parent, const StepInfo& stepInfo)
        : RunnerContext{ &parent }
        , stepInfo{ stepInfo }
    {
    }

    ContextManager::ContextManager(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
        : programContext{ std::move(contextStorageFactory) }
    {}

    void ContextManager::StartFeature(const FeatureInfo& featureInfo)
    {
        featureContext.emplace(programContext, featureInfo);
    }

    void ContextManager::StopFeature()
    {
        featureContext = std::nullopt;
    }

    void ContextManager::StartRule(const RuleInfo& ruleInfo)
    {
        ruleContext.emplace(*featureContext, ruleInfo);
    }

    void ContextManager::StopRule()
    {
        ruleContext = std::nullopt;
    }

    void ContextManager::StartScenario(const ScenarioInfo& scenarioInfo)
    {
        scenarioContext.emplace(*featureContext, scenarioInfo);
    }

    void ContextManager::StopScenario()
    {
        scenarioContext = std::nullopt;
    }

    void ContextManager::StartStep(const StepInfo& stepInfo)
    {
        stepContext.emplace(CurrentContext(), stepInfo);
    }

    void ContextManager::StopStep()
    {
        stepContext.pop();
    }

    struct ProgramContext& ContextManager::ProgramContext()
    {
        return programContext;
    }

    const struct ProgramContext& ContextManager::ProgramContext() const
    {
        return programContext;
    }

    struct FeatureContext& ContextManager::FeatureContext()
    {
        return *featureContext;
    }

    struct RuleContext& ContextManager::RuleContext()
    {
        return *ruleContext;
    }

    struct ScenarioContext& ContextManager::ScenarioContext()
    {
        return *scenarioContext;
    }

    struct StepContext& ContextManager::StepContext()
    {
        return stepContext.top();
    }

    RunnerContext& ContextManager::CurrentContext()
    {
        if (!stepContext.empty())
            return stepContext.top();
        if (scenarioContext)
            return *scenarioContext;
        if (featureContext)
            return *featureContext;
        return programContext;
    }
}
