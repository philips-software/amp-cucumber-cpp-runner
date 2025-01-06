#ifndef ENGINE_TESTEXECUTION_HPP
#define ENGINE_TESTEXECUTION_HPP

#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FailureHandler.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/HookExecutor.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include "cucumber_cpp/library/util/Immoveable.hpp"
#include <variant>
#include <vector>

namespace cucumber_cpp::library::engine
{
    struct TestExecution
    {
        struct ProgramScope;
        struct FeatureScope;
        struct RuleScope;
        struct ScenarioScope;

        struct Policy;

        [[nodiscard]] virtual ProgramScope StartRun() = 0;
        [[nodiscard]] virtual FeatureScope StartFeature(const cucumber_cpp::library::engine::FeatureInfo& featureInfo) = 0;
        [[nodiscard]] virtual RuleScope StartRule(const cucumber_cpp::library::engine::RuleInfo& ruleInfo) = 0;
        [[nodiscard]] virtual ScenarioScope StartScenario(const cucumber_cpp::library::engine::ScenarioInfo& scenarioInfo) = 0;

        virtual void RunStep(const cucumber_cpp::library::engine::StepInfo& stepInfo) = 0;
    };

    struct TestExecution::ProgramScope : library::util::Immoveable
    {
        ProgramScope(cucumber_cpp::library::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler, HookExecutor& hookExecution);

    private:
        cucumber_cpp::library::engine::ContextManager& contextManager;
        report::ReportForwarder& reportHandler;

        report::ReportForwarder::ProgramScope scopedProgramReport;
        HookExecutor::ProgramScope scopedProgramHook;
    };

    struct TestExecution::FeatureScope : library::util::Immoveable
    {
        FeatureScope(cucumber_cpp::library::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler, HookExecutor& hookExecution, const cucumber_cpp::library::engine::FeatureInfo& featureInfo);

    private:
        cucumber_cpp::library::engine::ContextManager::ScopedFeautureContext scopedFeatureContext;
        report::ReportForwarder::FeatureScope scopedFeatureReport;
        HookExecutor::FeatureScope scopedFeatureHook;

        cucumber_cpp::library::engine::ContextManager& contextManager;
        report::ReportForwarder& reportHandler;
    };

    struct TestExecution::RuleScope : library::util::Immoveable
    {
        RuleScope(cucumber_cpp::library::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler, const cucumber_cpp::library::engine::RuleInfo& ruleInfo);

    private:
        cucumber_cpp::library::engine::ContextManager::ScopedRuleContext scopedRuleContext;
        report::ReportForwarder::RuleScope scopedRuleReport;

        cucumber_cpp::library::engine::ContextManager& contextManager;
        report::ReportForwarder& reportHandler;
    };

    struct TestExecution::ScenarioScope : library::util::Immoveable
    {
        ScenarioScope(cucumber_cpp::library::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler, HookExecutor& hookExecution, const cucumber_cpp::library::engine::ScenarioInfo& scenarioInfo);

    private:
        cucumber_cpp::library::engine::ContextManager::ScopedScenarioContext scopedScenarioContext;
        report::ReportForwarder::ScenarioScope scopedScenarioReport;
        HookExecutor::ScenarioScope scopedScenarioHook;

        cucumber_cpp::library::engine::ContextManager& contextManager;
        report::ReportForwarder& reportHandler;
    };

    struct TestExecution::Policy
    {
        virtual void RunStep(cucumber_cpp::library::engine::ContextManager& contextManager, const StepMatch& stepMatch) const = 0;
    };

    struct DryRunPolicy : TestExecution::Policy
    {
        void RunStep(cucumber_cpp::library::engine::ContextManager& contextManager, const StepMatch& stepMatch) const override;
    };

    struct ExecuteRunPolicy : TestExecution::Policy
    {
        void RunStep(cucumber_cpp::library::engine::ContextManager& contextManager, const StepMatch& stepMatch) const override;
    };

    extern const DryRunPolicy dryRunPolicy;
    extern const ExecuteRunPolicy executeRunPolicy;

    struct TestExecutionImpl : TestExecution
    {
        TestExecutionImpl(cucumber_cpp::library::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler, HookExecutor& hookExecution, const Policy& executionPolicy = executeRunPolicy);
        ProgramScope StartRun() override;
        FeatureScope StartFeature(const cucumber_cpp::library::engine::FeatureInfo& featureInfo) override;
        RuleScope StartRule(const cucumber_cpp::library::engine::RuleInfo& ruleInfo) override;
        ScenarioScope StartScenario(const cucumber_cpp::library::engine::ScenarioInfo& scenarioInfo) override;

        void RunStep(const cucumber_cpp::library::engine::StepInfo& stepInfo) override;

    private:
        void RunStepMatch(std::monostate);
        void RunStepMatch(const std::vector<StepMatch>&);
        void RunStepMatch(const StepMatch& stepMatch);

        cucumber_cpp::library::engine::ContextManager& contextManager;
        report::ReportForwarder& reportHandler;
        HookExecutor& hookExecution;

        const Policy& executionPolicy;

        TestAssertionHandlerImpl testAssertionHandler{ contextManager, reportHandler };
    };
}

#endif
