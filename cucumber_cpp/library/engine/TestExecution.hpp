#ifndef ENGINE_TESTEXECUTION_HPP
#define ENGINE_TESTEXECUTION_HPP

#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/Events.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/HookExecutor.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/engine/TestFailureHandler.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include <variant>
#include <vector>

namespace cucumber_cpp::library::engine
{
    struct TestExecution
    {
        virtual void StartRun() = 0;
        virtual void EndRun() = 0;

        virtual void StartFeature(const ::cucumber_cpp::engine::FeatureInfo& featureInfo) = 0;
        virtual void EndFeature() = 0;

        virtual void StartRule(const ::cucumber_cpp::engine::RuleInfo& ruleInfo) = 0;
        virtual void EndRule() = 0;

        virtual void StartScenario(const ::cucumber_cpp::engine::ScenarioInfo& scenarioInfo) = 0;
        virtual void EndScenario() = 0;

        virtual void RunStep(const ::cucumber_cpp::engine::StepInfo& stepInfo) = 0;
    };

    struct TestExecutionImpl : TestExecution
    {
        TestExecutionImpl(::cucumber_cpp::engine::ContextManager& contextManager, report::ReportHandlerV2& reportHandler, HookExecutor& hookExecution, EventSubjects& eventsSubjects)
            : contextManager{ contextManager }
            , reportHandler{ reportHandler }
            , hookExecution{ hookExecution }
            , eventsSubjects{ eventsSubjects }
        {}

        void StartRun() override;
        void EndRun() override;

        void StartFeature(const ::cucumber_cpp::engine::FeatureInfo& featureInfo) override;
        void EndFeature() override;

        void StartRule(const ::cucumber_cpp::engine::RuleInfo& ruleInfo) override;
        void EndRule() override;

        void StartScenario(const ::cucumber_cpp::engine::ScenarioInfo& scenarioInfo) override;
        void EndScenario() override;

        void RunStep(const ::cucumber_cpp::engine::StepInfo& stepInfo) override;

        // void StepFailure() override;
        // void StepError() override;
        // void HookFailure() override;
        // void HookError() override;

    private:
        void RunStepMatch(std::monostate);
        void RunStepMatch(const std::vector<StepMatch>&);
        void RunStepMatch(const StepMatch& stepMatch);

        ::cucumber_cpp::engine::ContextManager& contextManager;
        report::ReportHandlerV2& reportHandler;
        HookExecutor& hookExecution;
        EventSubjects& eventsSubjects;

        TestAssertionHandlerImpl testAssertionHandler{ contextManager, reportHandler };
    };
}

#endif
