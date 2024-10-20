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

namespace cucumber_cpp::engine
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

    ContextManager::ContextManager(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
    {
        programContext = std::make_unique<struct ProgramContext>(std::move(contextStorageFactory));
    }

    cucumber_cpp::engine::ProgramContext& ContextManager::ProgramContext()
    {
        return *programContext;
    }

    cucumber_cpp::engine::ProgramContext& ContextManager::ProgramContext() const
    {
        return *programContext;
    }

    void ContextManager::CreateFeatureContext(const FeatureInfo& featureInfo)
    {
        featureContext = std::make_unique<decltype(featureContext)::element_type>(*programContext, featureInfo);
    }

    void ContextManager::DisposeFeatureContext()
    {
        featureContext.reset();
    }

    FeatureContext& ContextManager::FeatureContext()
    {
        return GetOrThrow(featureContext, "FeatureContext");
    }

    void ContextManager::CreateRuleContext(const RuleInfo& ruleInfo)
    {
        ruleContext = std::make_unique<decltype(ruleContext)::element_type>(*featureContext, ruleInfo);
    }

    void ContextManager::DisposeRuleContext()
    {
        ruleContext.reset();
    }

    RuleContext& ContextManager::RuleContext()
    {
        return GetOrThrow(ruleContext, "RuleContext");
    }

    void ContextManager::CreateScenarioContext(const ScenarioInfo& scenarioInfo)
    {
        if (ruleContext)
            scenarioContext = std::make_unique<decltype(scenarioContext)::element_type>(*ruleContext, scenarioInfo);
        else
            scenarioContext = std::make_unique<decltype(scenarioContext)::element_type>(*featureContext, scenarioInfo);
    }

    void ContextManager::DisposeScenarioContext()
    {
        scenarioContext.reset();
    }

    ScenarioContext& ContextManager::ScenarioContext()
    {
        return GetOrThrow(scenarioContext, "ScenarioContext");
    }

    void ContextManager::CreateStepContext(const StepInfo& stepInfo)
    {
        stepContext.push(std::make_unique<decltype(stepContext)::value_type::element_type>(*scenarioContext, stepInfo));
    }

    void ContextManager::DisposeStepContext()
    {
        stepContext.pop();
    }

    StepContext& ContextManager::StepContext()
    {
        if (stepContext.empty())
            throw ContextNotAvailable{ "StepContext not available" };

        return *stepContext.top();
    }
}
