#include "cucumber_cpp/library/engine/TestExecution.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include <variant>
#include <vector>

namespace cucumber_cpp::library::engine
{
    void TestExecutionImpl::StartRun()
    {
        hookExecution.BeforeAll();
    }

    void TestExecutionImpl::EndRun()
    {
        hookExecution.AfterAll();
    }

    void TestExecutionImpl::StartFeature(const ::cucumber_cpp::engine::FeatureInfo& featureInfo)
    {
        contextManager.CreateFeatureContext(featureInfo);
        reportHandler.FeatureStart(featureInfo);
        eventsSubjects.NotifyFeatureStarted();
        hookExecution.BeforeFeature();
    }

    void TestExecutionImpl::EndFeature()
    {
        hookExecution.AfterFeature();
        eventsSubjects.NotifyFeatureFinished();
        reportHandler.FeatureEnd(contextManager.FeatureContext().ExecutionStatus(), contextManager.FeatureContext().info, {});
        contextManager.DisposeFeatureContext();
    }

    void TestExecutionImpl::StartRule(const ::cucumber_cpp::engine::RuleInfo& ruleInfo)
    {
        contextManager.CreateRuleContext(ruleInfo);
        reportHandler.RuleStart(ruleInfo);
    }

    void TestExecutionImpl::EndRule()
    {
        reportHandler.RuleEnd(contextManager.RuleContext().ExecutionStatus(), contextManager.RuleContext().info, {});
        contextManager.DisposeRuleContext();
    }

    void TestExecutionImpl::StartScenario(const ::cucumber_cpp::engine::ScenarioInfo& scenarioInfo)
    {
        contextManager.CreateScenarioContext(scenarioInfo);
        reportHandler.ScenarioStart(scenarioInfo);
        eventsSubjects.NotifyScenarioStarted();
        hookExecution.BeforeScenario();
    }

    void TestExecutionImpl::EndScenario()
    {
        hookExecution.AfterScenario();
        eventsSubjects.NotifyScenarioFinished();
        reportHandler.ScenarioEnd(contextManager.ScenarioContext().ExecutionStatus(), contextManager.ScenarioContext().info, {});
        contextManager.DisposeScenarioContext();
    }

    void TestExecutionImpl::RunStep(const ::cucumber_cpp::engine::StepInfo& stepInfo)
    {
        contextManager.CreateStepContext(stepInfo);
        reportHandler.StepStart(stepInfo);
        eventsSubjects.NotifyStepStarted();
        hookExecution.BeforeStep();

        if (contextManager.ScenarioContext().ExecutionStatus() == ::cucumber_cpp::engine::Result::passed)
        {
            std::visit([this](const auto& value)
                {
                    RunStepMatch(value);
                },
                stepInfo.StepMatch());
        }

        hookExecution.AfterStep();
        eventsSubjects.NotifyStepFinished();
        reportHandler.StepEnd(contextManager.StepContext().ExecutionStatus(), contextManager.StepContext().info, {});
        contextManager.DisposeStepContext();
    }

    void TestExecutionImpl::RunStepMatch(std::monostate)
    {
        contextManager.StepContext().ExecutionStatus(::cucumber_cpp::engine::Result::undefined);
    }

    void TestExecutionImpl::RunStepMatch(const std::vector<StepMatch>&)
    {
        contextManager.StepContext().ExecutionStatus(::cucumber_cpp::engine::Result::ambiguous);
    }

    void TestExecutionImpl::RunStepMatch(const StepMatch& stepMatch)
    {
        auto& stepContext = contextManager.StepContext();
        auto& scenarioContext = contextManager.ScenarioContext();
        stepMatch.factory(scenarioContext, stepContext.info.Table())->Execute(stepMatch.matches);
    }
}
