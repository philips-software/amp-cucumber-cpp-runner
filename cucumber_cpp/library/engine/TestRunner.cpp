
#include "cucumber_cpp/library/engine/TestRunner.hpp"
#include "cucumber_cpp/library/engine/FeatureFactory.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/TestExecution.hpp"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::engine
{
    TestRunner* TestRunner::instance = nullptr;

    TestRunner::TestRunner()
    {
        TestRunner::instance = this;
    }

    TestRunner::~TestRunner()
    {
        TestRunner::instance = nullptr;
    }

    TestRunner& TestRunner::Instance()
    {
        return *instance;
    }

    TestRunnerImpl::TestRunnerImpl(cucumber_cpp::library::engine::TestExecution& testExecution)
        : testExecution{ testExecution }
    {
    }

    void TestRunnerImpl::Run(const std::vector<std::unique_ptr<FeatureInfo>>& feature)
    {
        auto scope = testExecution.StartRun();

        for (const auto& featurePtr : feature)
            RunFeature(*featurePtr);
    }

    void TestRunnerImpl::NestedStep(StepType type, std::string step)
    {
        const auto nestedStep = FeatureTreeFactory::CreateStepInfo(type, std::move(step), *currentScenario, 0, 0, {});
        testExecution.RunStep(*nestedStep);
    }

    void TestRunnerImpl::RunFeature(const FeatureInfo& feature)
    {
        if (feature.Rules().empty() && feature.Scenarios().empty())
            return;

        const auto featureScope = testExecution.StartFeature(feature);

        RunRules(feature.Rules());
        RunScenarios(feature.Scenarios());
    }

    void TestRunnerImpl::RunRule(const std::unique_ptr<RuleInfo>& rule)
    {
        const auto ruleScope = testExecution.StartRule(*rule);

        RunScenarios(rule->Scenarios());
    }

    void TestRunnerImpl::RunRules(const std::vector<std::unique_ptr<RuleInfo>>& rules)
    {
        for (const auto& rule : rules)
            RunRule(rule);
    }

    void TestRunnerImpl::RunScenario(const std::unique_ptr<ScenarioInfo>& scenario)
    {
        const auto scenarioScope = testExecution.StartScenario(*scenario);

        currentScenario = scenario.get();

        ExecuteSteps(*scenario);
    }

    void TestRunnerImpl::RunScenarios(const std::vector<std::unique_ptr<ScenarioInfo>>& scenarios)
    {
        for (const auto& scenario : scenarios)
            RunScenario(scenario);
    }

    void TestRunnerImpl::ExecuteSteps(const ScenarioInfo& scenario)
    {
        for (const auto& step : scenario.Children())
            testExecution.RunStep(*step);
    }
}
