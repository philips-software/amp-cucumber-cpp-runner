
#include "cucumber_cpp/library/engine/TestRunner.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/engine/TestExecution.hpp"
#include <memory>
#include <vector>

namespace cucumber_cpp::engine
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

    TestRunnerImpl::TestRunnerImpl(::cucumber_cpp::library::engine::TestExecution& testExecution)
        : testExecution{ testExecution }
    {
    }

    void TestRunnerImpl::Run(const std::vector<std::unique_ptr<FeatureInfo>>& feature)
    {
        testExecution.StartRun();

        for (const auto& featurePtr : feature)
            RunFeature(*featurePtr);

        testExecution.EndRun();
    }

    void TestRunnerImpl::RunFeature(const FeatureInfo& feature)
    {
        if (feature.Rules().empty() && feature.Scenarios().empty())
            return;

        testExecution.StartFeature(feature);

        RunRules(feature.Rules());
        RunScenarios(feature.Scenarios());

        testExecution.EndFeature();
    }

    void TestRunnerImpl::RunRules(const std::vector<std::unique_ptr<RuleInfo>>& rules)
    {
        for (const auto& rule : rules)
        {
            testExecution.StartRule(*rule);

            RunScenarios(rule->Scenarios());

            testExecution.EndRule();
        }
    }

    void TestRunnerImpl::RunScenarios(const std::vector<std::unique_ptr<ScenarioInfo>>& scenarios)
    {
        for (const auto& scenario : scenarios)
        {
            testExecution.StartScenario(*scenario);

            ExecuteSteps(*scenario);

            testExecution.EndScenario();
        }
    }

    void TestRunnerImpl::ExecuteSteps(const ScenarioInfo& scenario)
    {
        for (const auto& step : scenario.Children())
        {
            testExecution.RunStep(*step);
        }
    }

}
