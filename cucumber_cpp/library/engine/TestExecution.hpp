#ifndef ENGINE_TESTEXECUTION_HPP
#define ENGINE_TESTEXECUTION_HPP

#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FailureHandler.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/HookExecutor.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include "cucumber_cpp/library/util/Immoveable.hpp"
#include <optional>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::engine
{
    struct TestExecution
    {
    protected:
        ~TestExecution() = default;

    public:
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

        Result ExecutionStatus() const;

    private:
        cucumber_cpp::library::engine::ContextManager& contextManager;
        report::ReportForwarder::ProgramScope scopedProgramReport;
        HookExecutor::ProgramScope scopedProgramHook;
    };

    struct TestExecution::FeatureScope : library::util::Immoveable
    {
        FeatureScope(cucumber_cpp::library::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler, HookExecutor& hookExecution, const cucumber_cpp::library::engine::FeatureInfo& featureInfo);

        Result ExecutionStatus() const;

    private:
        cucumber_cpp::library::engine::ContextManager::ScopedFeatureContext scopedFeatureContext;
        report::ReportForwarder::FeatureScope scopedFeatureReport;
        HookExecutor::FeatureScope scopedFeatureHook;
    };

    struct TestExecution::RuleScope : library::util::Immoveable
    {
        RuleScope(cucumber_cpp::library::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler, const cucumber_cpp::library::engine::RuleInfo& ruleInfo);

    private:
        cucumber_cpp::library::engine::ContextManager::ScopedRuleContext scopedRuleContext;
        report::ReportForwarder::RuleScope scopedRuleReport;
    };

    struct TestExecution::ScenarioScope : library::util::Immoveable
    {
        ScenarioScope(cucumber_cpp::library::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler, HookExecutor& hookExecution, const cucumber_cpp::library::engine::ScenarioInfo& scenarioInfo);

    private:
        cucumber_cpp::library::engine::ContextManager::ScopedScenarioContext scopedScenarioContext;
        report::ReportForwarder::ScenarioScope scopedScenarioReport;
        HookExecutor::ScenarioScope scopedScenarioHook;
    };

    struct TestExecution::Policy
    {
    protected:
        ~Policy() = default;

    public:
        virtual void RunStep(cucumber_cpp::library::engine::ContextManager& contextManager, const StepRegistry::StepMatch& stepMatch) const = 0;
    };

    struct DryRunPolicy : TestExecution::Policy
    {
        virtual ~DryRunPolicy() = default;

        void RunStep(cucumber_cpp::library::engine::ContextManager& contextManager, const StepRegistry::StepMatch& stepMatch) const override;
    };

    struct ExecuteRunPolicy : TestExecution::Policy
    {
        virtual ~ExecuteRunPolicy() = default;

        void RunStep(cucumber_cpp::library::engine::ContextManager& contextManager, const StepRegistry::StepMatch& stepMatch) const override;
    };

    extern const DryRunPolicy dryRunPolicy;
    extern const ExecuteRunPolicy executeRunPolicy;

    struct TestExecutionImpl : TestExecution
    {
        TestExecutionImpl(cucumber_cpp::library::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler, HookExecutor& hookExecution, const Policy& executionPolicy = executeRunPolicy);
        virtual ~TestExecutionImpl() = default;

        ProgramScope StartRun() override;
        FeatureScope StartFeature(const cucumber_cpp::library::engine::FeatureInfo& featureInfo) override;
        RuleScope StartRule(const cucumber_cpp::library::engine::RuleInfo& ruleInfo) override;
        ScenarioScope StartScenario(const cucumber_cpp::library::engine::ScenarioInfo& scenarioInfo) override;

        void RunStep(const cucumber_cpp::library::engine::StepInfo& stepInfo) override;

    private:
        void RunStepMatch(std::monostate);
        void RunStepMatch(const std::vector<StepRegistry::StepMatch>&);
        void RunStepMatch(const StepRegistry::StepMatch& stepMatch);

        cucumber_cpp::library::engine::ContextManager& contextManager;
        report::ReportForwarder& reportHandler;
        HookExecutor& hookExecution;

        const Policy& executionPolicy;

        TestAssertionHandlerImpl testAssertionHandler{ contextManager, reportHandler };
    };
}

#endif
