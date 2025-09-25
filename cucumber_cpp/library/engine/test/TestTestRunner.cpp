
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FeatureFactory.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/HookExecutor.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/engine/TestExecution.hpp"
#include "cucumber_cpp/library/engine/TestRunner.hpp"
#include "cucumber_cpp/library/engine/test_helper/TemporaryFile.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include "cucumber_cpp/library/report/StdOutReport.hpp"
#include "gmock/gmock.h"
#include <gtest/gtest-spi.h>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace cucumber_cpp::library::engine
{
    struct TestExecutionMockInstance : TestExecution
    {
        TestExecutionMockInstance()
        {
            reporters.Add("console", std::make_unique<report::StdOutReport>());
            reporters.Use("console");
        }

        virtual ~TestExecutionMockInstance() = default;

        MOCK_METHOD(void, StartRunMock, ());
        MOCK_METHOD(void, StartFeatureMock, (const FeatureInfo& featureInfo));
        MOCK_METHOD(void, StartRuleMock, (const RuleInfo& ruleInfo));
        MOCK_METHOD(void, StartScenarioMock, (const ScenarioInfo& scenarioInfo));

        MOCK_METHOD(void, RunStepMock, (const StepInfo& stepInfo));

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

        ContextManager contextManager{ std::make_shared<ContextStorageFactoryImpl>() };

        HookExecutorImpl hookExecutor{ contextManager };
        report::ReportForwarderImpl reporters{ contextManager };

        TestExecutionImpl testExecutionImpl{ contextManager, reporters, hookExecutor };
    };

    struct TestTestRunner : testing::Test
    {
        cucumber_expression::ParameterRegistry parameterRegistry;
        StepRegistry stepRegistry{ parameterRegistry };
        FeatureTreeFactory featureTreeFactory{ stepRegistry };

        testing::StrictMock<TestExecutionMockInstance> testExecutionMock;
        TestRunnerImpl runner{ featureTreeFactory, testExecutionMock };

        std::vector<std::unique_ptr<FeatureInfo>> features;

        ContextManager& contextManager = testExecutionMock.contextManager;
    };

    TEST_F(TestTestRunner, StartProgramContext)
    {
        EXPECT_CALL(testExecutionMock, StartRunMock());

        runner.Run({});
    }

    TEST_F(TestTestRunner, StartFeatureAndScenarioContext)
    {
        EXPECT_CALL(testExecutionMock, StartRunMock);
        EXPECT_CALL(testExecutionMock, StartFeatureMock);
        EXPECT_CALL(testExecutionMock, StartScenarioMock);

        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };
        tmp << "Feature: Test feature\n"
               "  Scenario: Test scenario\n";
        features.push_back(featureTreeFactory.Create(tmp.Path(), ""));

        runner.Run(features);
    }

    TEST_F(TestTestRunner, DontRunEmptyFeature)
    {
        EXPECT_CALL(testExecutionMock, StartRunMock);
        EXPECT_CALL(testExecutionMock, StartFeatureMock).Times(0);

        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };
        tmp << "Feature: Test feature\n";
        features.push_back(featureTreeFactory.Create(tmp.Path(), ""));

        runner.Run(features);
    }

    TEST_F(TestTestRunner, StartFeatureContextForEveryFeature)
    {
        EXPECT_CALL(testExecutionMock, StartRunMock);
        EXPECT_CALL(testExecutionMock, StartFeatureMock);
        EXPECT_CALL(testExecutionMock, StartScenarioMock).Times(2);

        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };
        tmp << "Feature: Test feature\n"
               "  Scenario: Test scenario\n"
               "  Scenario: Test scenario\n";

        features.push_back(featureTreeFactory.Create(tmp.Path(), ""));

        runner.Run(features);
    }

    TEST_F(TestTestRunner, StartScenarioContextForEveryScenario)
    {
        EXPECT_CALL(testExecutionMock, StartRunMock);
        EXPECT_CALL(testExecutionMock, StartFeatureMock).Times(2);
        EXPECT_CALL(testExecutionMock, StartScenarioMock).Times(2);

        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };
        tmp << "Feature: Test feature\n"
               "  Scenario: Test scenario\n";

        features.push_back(featureTreeFactory.Create(tmp.Path(), ""));
        features.push_back(featureTreeFactory.Create(tmp.Path(), ""));

        runner.Run(features);
    }

    TEST_F(TestTestRunner, StartRuleAndScenarioForEveryRuleAndScenario)
    {
        EXPECT_CALL(testExecutionMock, StartRunMock);
        EXPECT_CALL(testExecutionMock, StartFeatureMock).Times(2);
        EXPECT_CALL(testExecutionMock, StartRuleMock).Times(4);
        EXPECT_CALL(testExecutionMock, StartScenarioMock).Times(4);

        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };
        tmp << "Feature: Test feature\n"
               "  Rule: Test rule\n"
               "    Scenario: Test scenario\n"
               "  Rule: Test rule\n"
               "    Scenario: Test scenario\n";

        features.push_back(featureTreeFactory.Create(tmp.Path(), ""));
        features.push_back(featureTreeFactory.Create(tmp.Path(), ""));

        runner.Run(features);
    }

    TEST_F(TestTestRunner, RunEveryStep)
    {
        EXPECT_CALL(testExecutionMock, StartRunMock);
        EXPECT_CALL(testExecutionMock, StartFeatureMock);
        EXPECT_CALL(testExecutionMock, StartRuleMock).Times(1);
        EXPECT_CALL(testExecutionMock, StartScenarioMock).Times(2);
        EXPECT_CALL(testExecutionMock, RunStepMock).Times(4);

        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };
        tmp << "Feature: Test feature\n"
               "  Scenario: Test scenario\n"
               "    Given I have a step\n"
               "    Given I have a step\n"
               "  Rule: Test rule\n"
               "    Scenario: Test scenario\n"
               "      Given I have a step\n"
               "      Given I have a step\n";

        features.push_back(featureTreeFactory.Create(tmp.Path(), ""));
        runner.Run(features);
    }

    TEST_F(TestTestRunner, RunNestedSteps)
    {
        EXPECT_CALL(testExecutionMock, StartRunMock);
        EXPECT_CALL(testExecutionMock, StartFeatureMock);
        EXPECT_CALL(testExecutionMock, StartScenarioMock).Times(1);
        EXPECT_CALL(testExecutionMock, RunStepMock).Times(3);

        auto tmp = test_helper::TemporaryFile{ "tmpfile.feature" };
        tmp << "Feature: Test feature\n"
               "  Scenario: Test scenario\n"
               "    When I call a nested step\n"
               "    Then the nested step was called\n";

        features.push_back(featureTreeFactory.Create(tmp.Path(), ""));

        ASSERT_THAT(contextManager.ProgramContext().ExecutionStatus(), testing::Eq(Result::passed));

        runner.Run(features);

        ASSERT_THAT(contextManager.ProgramContext().ExecutionStatus(), testing::Eq(Result::passed));
    }
}
