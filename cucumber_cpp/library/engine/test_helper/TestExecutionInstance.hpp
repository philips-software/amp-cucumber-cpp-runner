#ifndef TEST_HELPER_TESTEXECUTIONINSTANCE_HPP
#define TEST_HELPER_TESTEXECUTIONINSTANCE_HPP

#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/HookExecutor.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/engine/TestExecution.hpp"
#include "cucumber_cpp/library/engine/test_helper/ContextManagerInstance.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include "gmock/gmock.h"

namespace cucumber_cpp::library::engine::test_helper
{
    struct TestExecutionMockInstance : TestExecution
    {
        virtual ~TestExecutionMockInstance() = default;

        MOCK_METHOD(void, StartRunMock, ());
        MOCK_METHOD(void, StartFeatureMock, (const FeatureInfo& featureInfo));
        MOCK_METHOD(void, StartRuleMock, (const RuleInfo& ruleInfo));
        MOCK_METHOD(void, StartScenarioMock, (const ScenarioInfo& scenarioInfo));

        MOCK_METHOD(void, RunStepMock, (const StepInfo& stepInfo));

    private:
        [[nodiscard]] ProgramScope StartRun() override
        {
            StartRunMock();
            return testExecutionImpl.StartRun();
        }

        [[nodiscard]] FeatureScope StartFeature(const cucumber_cpp::library::engine::FeatureInfo& featureInfo) override
        {
            StartFeatureMock(featureInfo);
            return testExecutionImpl.StartFeature(featureInfo);
        }

        [[nodiscard]] RuleScope StartRule(const cucumber_cpp::library::engine::RuleInfo& ruleInfo) override
        {
            StartRuleMock(ruleInfo);
            return testExecutionImpl.StartRule(ruleInfo);
        }

        [[nodiscard]] ScenarioScope StartScenario(const cucumber_cpp::library::engine::ScenarioInfo& scenarioInfo) override
        {
            StartScenarioMock(scenarioInfo);
            return testExecutionImpl.StartScenario(scenarioInfo);
        }

        void RunStep(const cucumber_cpp::library::engine::StepInfo& stepInfo) override
        {
            RunStepMock(stepInfo);
            testExecutionImpl.RunStep(stepInfo);
        }

        ContextManagerInstance contextManager;
        HookExecutorImpl hookExecutor{ contextManager };
        report::ReportForwarderImpl reporters{ contextManager };

        TestExecutionImpl testExecutionImpl{ contextManager, reporters, hookExecutor };
    };
}

#endif
