#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/TraceTime.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include <memory>
#include <string>
#include <utility>

namespace cucumber_cpp::library::engine
{
    auto& GetOrThrow(auto& ptr, std::string typeName)
    {
        if (ptr)
            return *ptr;

        throw ContextNotAvailable{ typeName + " not available" };
    }

    CurrentContext::CurrentContext(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
        : Context{ std::move(contextStorageFactory) }
    {
        Start();
    }

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

    ContextManager::ScopedFeautureContext::ScopedFeautureContext(ContextManager& contextManager)
        : contextManager{ contextManager }
    {}

    ContextManager::ScopedFeautureContext ::~ScopedFeautureContext()
    {
        contextManager.DisposeFeatureContext();
    }

    ContextManager::ScopedRuleContext::ScopedRuleContext(ContextManager& contextManager)
        : contextManager{ contextManager }
    {}

    ContextManager::ScopedRuleContext ::~ScopedRuleContext()
    {
        contextManager.DisposeRuleContext();
    }

    ContextManager::ScopedScenarioContext::ScopedScenarioContext(ContextManager& contextManager)
        : contextManager{ contextManager }
    {}

    ContextManager::ScopedScenarioContext ::~ScopedScenarioContext()
    {
        contextManager.DisposeScenarioContext();
    }

    ContextManager::ScopedStepContext::ScopedStepContext(ContextManager& contextManager)
        : contextManager{ contextManager }
    {}

    ContextManager::ScopedStepContext ::~ScopedStepContext()
    {
        contextManager.DisposeStepContext();
    }

    ContextManager::ContextManager(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
    {
        programContext = std::make_shared<struct ProgramContext>(std::move(contextStorageFactory));
        runnerContext.push(programContext);
    }

    cucumber_cpp::library::engine::ProgramContext& ContextManager::ProgramContext()
    {
        return *programContext;
    }

    cucumber_cpp::library::engine::ProgramContext& ContextManager::ProgramContext() const
    {
        return *programContext;
    }

    ContextManager::ScopedFeautureContext ContextManager::CreateFeatureContext(const FeatureInfo& featureInfo)
    {
        featureContext = std::make_shared<decltype(featureContext)::element_type>(*programContext, featureInfo);
        runnerContext.push(featureContext);

        return ScopedFeautureContext{ *this };
    }

    void ContextManager::DisposeFeatureContext()
    {
        runnerContext.pop();
        featureContext.reset();
    }

    FeatureContext& ContextManager::FeatureContext()
    {
        return GetOrThrow(featureContext, "FeatureContext");
    }

    ContextManager::ScopedRuleContext ContextManager::CreateRuleContext(const RuleInfo& ruleInfo)
    {
        ruleContext = std::make_shared<decltype(ruleContext)::element_type>(*featureContext, ruleInfo);
        runnerContext.push(ruleContext);

        return ScopedRuleContext{ *this };
    }

    void ContextManager::DisposeRuleContext()
    {
        runnerContext.pop();
        ruleContext.reset();
    }

    RuleContext& ContextManager::RuleContext()
    {
        return GetOrThrow(ruleContext, "RuleContext");
    }

    ContextManager::ScopedScenarioContext ContextManager::CreateScenarioContext(const ScenarioInfo& scenarioInfo)
    {
        scenarioContext = std::make_shared<decltype(scenarioContext)::element_type>(CurrentContext(), scenarioInfo);
        runnerContext.push(scenarioContext);

        return ScopedScenarioContext{ *this };
    }

    void ContextManager::DisposeScenarioContext()
    {
        runnerContext.pop();
        scenarioContext.reset();
    }

    ScenarioContext& ContextManager::ScenarioContext()
    {
        return GetOrThrow(scenarioContext, "ScenarioContext");
    }

    ContextManager::ScopedStepContext ContextManager::CreateStepContext(const StepInfo& stepInfo)
    {
        stepContext.push(std::make_shared<cucumber_cpp::library::engine::StepContext>(*scenarioContext, stepInfo));
        runnerContext.push(stepContext.top());

        return ScopedStepContext{ *this };
    }

    void ContextManager::DisposeStepContext()
    {
        runnerContext.pop();
        stepContext.pop();
    }

    StepContext& ContextManager::StepContext()
    {
        if (stepContext.empty())
            throw ContextNotAvailable{ "StepContext not available" };

        return *stepContext.top();
    }

    cucumber_cpp::library::engine::RunnerContext& ContextManager::CurrentContext()
    {
        return *runnerContext.top();
    }
}
