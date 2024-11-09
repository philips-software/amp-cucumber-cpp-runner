#include "cucumber_cpp/library/engine/TestExecution.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/HookExecutor.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include <memory>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::engine
{
    const DryRunPolicy dryRunPolicy;
    const ExecuteRunPolicy executeRunPolicy;

    TestExecution::ProgramScope::ProgramScope(HookExecutor& hookExecution)
        : scopedProgramHook{ hookExecution.BeforeAll() }
    {
    }

    TestExecution::FeatureScope::FeatureScope(cucumber_cpp::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler, HookExecutor& hookExecution, const ::cucumber_cpp::engine::FeatureInfo& featureInfo)
        : scopedFeatureContext{ contextManager.CreateFeatureContext(featureInfo) }
        , scopedFeatureReport{ reportHandler.FeatureStart() }
        , scopedFeatureHook{ hookExecution.FeatureStart() }
        , contextManager{ contextManager }
        , reportHandler{ reportHandler }
    {
    }

    TestExecution::RuleScope::RuleScope(cucumber_cpp::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler, const cucumber_cpp::engine::RuleInfo& ruleInfo)
        : scopedRuleContext{ contextManager.CreateRuleContext(ruleInfo) }
        , scopedRuleReport{ reportHandler.RuleStart() }
        , contextManager{ contextManager }
        , reportHandler{ reportHandler }
    {
    }

    TestExecution::ScenarioScope::ScenarioScope(cucumber_cpp::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler, HookExecutor& hookExecution, const cucumber_cpp::engine::ScenarioInfo& scenarioInfo)
        : scopedScenarioContext{ contextManager.CreateScenarioContext(scenarioInfo) }
        , scopedScenarioReport{ reportHandler.ScenarioStart() }
        , scopedScenarioHook{ hookExecution.ScenarioStart() }
        , contextManager{ contextManager }
        , reportHandler{ reportHandler }
    {
    }

    void DryRunPolicy::RunStep(cucumber_cpp::engine::ContextManager&, const StepMatch&) const
    {}

    void ExecuteRunPolicy::RunStep(cucumber_cpp::engine::ContextManager& contextManager, const StepMatch& stepMatch) const
    {
        auto& stepContext = contextManager.StepContext();
        auto& scenarioContext = contextManager.ScenarioContext();
        stepMatch.factory(scenarioContext, stepContext.info.Table())->Execute(stepMatch.matches);
    }

    TestExecutionImpl::TestExecutionImpl(cucumber_cpp::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler, HookExecutor& hookExecution, const Policy& executionPolicy)
        : contextManager{ contextManager }
        , reportHandler{ reportHandler }
        , hookExecution{ hookExecution }
        , executionPolicy{ executionPolicy }
    {}

    TestExecution::ProgramScope TestExecutionImpl::StartRun()
    {
        return ProgramScope{ hookExecution };
    }

    TestExecution::FeatureScope TestExecutionImpl::StartFeature(const ::cucumber_cpp::engine::FeatureInfo& featureInfo)
    {
        return FeatureScope{ contextManager, reportHandler, hookExecution, featureInfo };
    }

    TestExecution::RuleScope TestExecutionImpl::StartRule(const ::cucumber_cpp::engine::RuleInfo& ruleInfo)
    {
        return RuleScope{ contextManager, reportHandler, ruleInfo };
    }

    TestExecution::ScenarioScope TestExecutionImpl::StartScenario(const ::cucumber_cpp::engine::ScenarioInfo& scenarioInfo)
    {
        return ScenarioScope{ contextManager, reportHandler, hookExecution, scenarioInfo };
    }

    void TestExecutionImpl::RunStep(const ::cucumber_cpp::engine::StepInfo& stepInfo)
    {
        auto scopedContext = contextManager.CreateStepContext(stepInfo);
        if (contextManager.ScenarioContext().ExecutionStatus() == cucumber_cpp::engine::Result::passed)
        {
            const auto scopedStepReport = reportHandler.StepStart();
            const auto scopedStepHook = hookExecution.StepStart();

            std::visit([this](const auto& value)
                {
                    RunStepMatch(value);
                },
                stepInfo.StepMatch());
        }
        else
        {
            contextManager.StepContext().ExecutionStatus(cucumber_cpp::engine::Result::skipped);
            reportHandler.StepSkipped();
        }
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
        executionPolicy.RunStep(contextManager, stepMatch);
    }
}
