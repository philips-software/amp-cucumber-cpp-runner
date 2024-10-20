#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/TraceTime.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "gtest/gtest.h"
#include <filesystem>
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

    void RunnerContext::AppendFailure(testing::TestPartResult::Type type, std::filesystem::path srcfile, int line_num, std::string message)
    {
        failures.push_back(std::make_shared<Failure>(type, srcfile, line_num, message));
    }

    ProgramContext::ProgramContext(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
        : RunnerContext{ std::move(contextStorageFactory) }
    {
    }

    ContextManager::ContextManager(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
    {
        programContext = std::make_shared<struct ProgramContext>(std::move(contextStorageFactory));
        runnerContext.push(programContext);
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
        featureContext = std::make_shared<decltype(featureContext)::element_type>(*programContext, featureInfo);
        runnerContext.push(featureContext);
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

    void ContextManager::CreateRuleContext(const RuleInfo& ruleInfo)
    {
        ruleContext = std::make_shared<decltype(ruleContext)::element_type>(*featureContext, ruleInfo);
        runnerContext.push(ruleContext);
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

    void ContextManager::CreateScenarioContext(const ScenarioInfo& scenarioInfo)
    {
        scenarioContext = std::make_shared<decltype(scenarioContext)::element_type>(CurrentContext(), scenarioInfo);
        runnerContext.push(scenarioContext);
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

    void ContextManager::CreateStepContext(const StepInfo& stepInfo)
    {
        stepContext.push(std::make_shared<decltype(stepContext)::value_type::element_type>(*scenarioContext, stepInfo));
        runnerContext.push(stepContext.top());
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

    cucumber_cpp::engine::RunnerContext& ContextManager::CurrentContext()
    {
        return *runnerContext.top();
    }
}
