
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/HookExecutor.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include "cucumber_cpp/library/engine/TestExecution.hpp"
#include "cucumber_cpp/library/engine/TestRunner.hpp"
#include "cucumber_cpp/library/engine/test_helper/ContextManagerInstance.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace cucumber_cpp::library::engine
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

        library::engine::test_helper::ContextManagerInstance contextManager;
        HookExecutorImpl hookExecutor{ contextManager };
        report::ReportForwarderImpl reporters{ contextManager };

        TestExecutionImpl testExecutionImpl{ contextManager, reporters, hookExecutor };
    };

    struct TestTestRunner : testing::Test
    {
        TestTestRunner()
        {
            AddFeature(singleFeatureAndScenario);
            AddScenario(singleFeatureAndScenario);

            AddFeature(twoFeaturesAndSingleScenario);
            AddScenario(twoFeaturesAndSingleScenario);
            AddFeature(twoFeaturesAndSingleScenario);
            AddScenario(twoFeaturesAndSingleScenario);

            AddFeature(singleFeatureTwoScenarios);
            AddScenario(singleFeatureTwoScenarios);
            AddScenario(singleFeatureTwoScenarios);

            AddFeature(twoFeaturesAndTwoRules);
            AddRule(twoFeaturesAndTwoRules);
            AddScenarioToRule(twoFeaturesAndTwoRules);
            AddRule(twoFeaturesAndTwoRules);
            AddScenarioToRule(twoFeaturesAndTwoRules);
            AddFeature(twoFeaturesAndTwoRules);
            AddRule(twoFeaturesAndTwoRules);
            AddScenarioToRule(twoFeaturesAndTwoRules);
            AddRule(twoFeaturesAndTwoRules);
            AddScenarioToRule(twoFeaturesAndTwoRules);

            AddFeature(featureWithSteps);
            AddScenario(featureWithSteps);
            AddStep(featureWithSteps);
            AddStep(featureWithSteps);
            AddRule(featureWithSteps);
            AddScenarioToRule(featureWithSteps);
            AddStepToScenarioInRule(featureWithSteps);
            AddStepToScenarioInRule(featureWithSteps);
        }

    private:
        static void AddFeature(std::vector<std::unique_ptr<FeatureInfo>>& features)
        {
            std::set<std::string, std::less<>> noTags{};

            features.push_back(std::make_unique<FeatureInfo>(noTags, "title", "description", "path", 1, 1));
        }

        static void AddScenario(std::vector<std::unique_ptr<FeatureInfo>>& features)
        {
            std::set<std::string, std::less<>> noTags{};

            features.back()->Scenarios().push_back(std::make_unique<ScenarioInfo>(*features.back(), noTags, "title", "description", 1, 1));
        }

        static void AddRule(std::vector<std::unique_ptr<FeatureInfo>>& features)
        {
            std::set<std::string, std::less<>> noTags{};

            features.back()->Rules().push_back(std::make_unique<RuleInfo>(*features.back(), "id", "title", "description", 1, 1));
        }

        static void AddScenarioToRule(std::vector<std::unique_ptr<FeatureInfo>>& features)
        {
            std::set<std::string, std::less<>> noTags{};
            features.back()->Rules().back()->Scenarios().push_back(std::make_unique<ScenarioInfo>(*features.back(), noTags, "title", "description", 1, 1));
        }

        static void AddStep(std::vector<std::unique_ptr<FeatureInfo>>& features)
        {
            std::vector<std::vector<TableValue>> table{};
            features.back()->Scenarios().back()->Children().push_back(std::make_unique<StepInfo>(*features.back()->Scenarios().back(), "text", StepType::any, 1, 1, table));
        }

        static void AddStepToScenarioInRule(std::vector<std::unique_ptr<FeatureInfo>>& features)
        {
            std::vector<std::vector<TableValue>> table{};
            features.back()->Rules().back()->Scenarios().back()->Children().push_back(std::make_unique<StepInfo>(*features.back()->Rules().back()->Scenarios().back(), "text", StepType::any, 1, 1, table));
        }

    protected:
        std::vector<std::unique_ptr<FeatureInfo>> singleFeatureAndScenario;
        std::vector<std::unique_ptr<FeatureInfo>> twoFeaturesAndSingleScenario;
        std::vector<std::unique_ptr<FeatureInfo>> singleFeatureTwoScenarios;
        std::vector<std::unique_ptr<FeatureInfo>> twoFeaturesAndTwoRules;
        std::vector<std::unique_ptr<FeatureInfo>> featureWithSteps;
    };

    TEST_F(TestTestRunner, Construct)
    {
        TestExecutionMockInstance testExecutionMock;
        TestRunnerImpl runner{ testExecutionMock };
    }

    TEST_F(TestTestRunner, StartProgramContext)
    {
        testing::StrictMock<TestExecutionMockInstance> testExecutionMock;
        TestRunnerImpl runner{ testExecutionMock };

        EXPECT_CALL(testExecutionMock, StartRunMock());

        runner.Run({});
    }

    TEST_F(TestTestRunner, StartFeatureAndScenarioContext)
    {
        testing::StrictMock<TestExecutionMockInstance> testExecutionMock;
        TestRunnerImpl runner{ testExecutionMock };

        EXPECT_CALL(testExecutionMock, StartRunMock);
        EXPECT_CALL(testExecutionMock, StartFeatureMock);
        EXPECT_CALL(testExecutionMock, StartScenarioMock);

        runner.Run(singleFeatureAndScenario);
    }

    TEST_F(TestTestRunner, StartFeatureContextForEveryFeature)
    {
        testing::StrictMock<TestExecutionMockInstance> testExecutionMock;
        TestRunnerImpl runner{ testExecutionMock };

        EXPECT_CALL(testExecutionMock, StartRunMock);
        EXPECT_CALL(testExecutionMock, StartFeatureMock);
        EXPECT_CALL(testExecutionMock, StartScenarioMock).Times(2);

        runner.Run(singleFeatureTwoScenarios);
    }

    TEST_F(TestTestRunner, StartScenarioContextForEveryScenario)
    {
        testing::StrictMock<TestExecutionMockInstance> testExecutionMock;
        TestRunnerImpl runner{ testExecutionMock };

        EXPECT_CALL(testExecutionMock, StartRunMock);
        EXPECT_CALL(testExecutionMock, StartFeatureMock).Times(2);
        EXPECT_CALL(testExecutionMock, StartScenarioMock).Times(2);

        runner.Run(twoFeaturesAndSingleScenario);
    }

    TEST_F(TestTestRunner, StartRuleAndScenarioForEveryRuleAndScenario)
    {
        testing::StrictMock<TestExecutionMockInstance> testExecutionMock;
        TestRunnerImpl runner{ testExecutionMock };

        EXPECT_CALL(testExecutionMock, StartRunMock);
        EXPECT_CALL(testExecutionMock, StartFeatureMock).Times(2);
        EXPECT_CALL(testExecutionMock, StartRuleMock).Times(4);
        EXPECT_CALL(testExecutionMock, StartScenarioMock).Times(4);

        runner.Run(twoFeaturesAndTwoRules);
    }

    TEST_F(TestTestRunner, RunEveryStep)
    {
        testing::StrictMock<TestExecutionMockInstance> testExecutionMock;
        TestRunnerImpl runner{ testExecutionMock };

        EXPECT_CALL(testExecutionMock, StartRunMock);
        EXPECT_CALL(testExecutionMock, StartFeatureMock);
        EXPECT_CALL(testExecutionMock, StartRuleMock).Times(1);
        EXPECT_CALL(testExecutionMock, StartScenarioMock).Times(2);
        EXPECT_CALL(testExecutionMock, RunStepMock).Times(4);

        runner.Run(featureWithSteps);
    }
}
