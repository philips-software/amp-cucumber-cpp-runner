
#include "cucumber_cpp/library/engine/TestRunner.hpp"
#include "cucumber_cpp/library/BodyMacro.hpp"
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
    TestRunner* TestRunner::instance = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

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

    void TestRunnerImpl::Run(const std::vector<std::unique_ptr<FeatureInfo>>& features)
    {
        auto runFeatures = [this, &features]
        {
            auto scope = testExecution.StartRun();

            for (const auto& featurePtr : features)
                RunFeature(*featurePtr);
        };

        ASSERT_NO_THROW(runFeatures());
    }

    void TestRunnerImpl::NestedStep(StepType type, std::string step)
    {
        const auto nestedStep = FeatureTreeFactory::CreateStepInfo(type, std::move(step), *currentScenario, 0, 0, {});
        testExecution.RunStep(*nestedStep);
    }

    void TestRunnerImpl::RunFeature(const FeatureInfo& feature)
    {
        auto runFeature = [this, &feature]
        {
            if (feature.Rules().empty() && feature.Scenarios().empty())
                return;

            const auto featureScope = testExecution.StartFeature(feature);

            RunRules(feature.Rules());
            RunScenarios(feature.Scenarios());
        };
        ASSERT_NO_THROW(runFeature());
    }

    void TestRunnerImpl::RunRule(const RuleInfo& rule)
    {
        auto runRule = [this, &rule]
        {
            const auto ruleScope = testExecution.StartRule(rule);

            RunScenarios(rule.Scenarios());
        };
        ASSERT_NO_THROW(runRule());
    }

    void TestRunnerImpl::RunRules(const std::vector<std::unique_ptr<RuleInfo>>& rules)
    {
        for (const auto& rule : rules)
            RunRule(*rule);
    }

    void TestRunnerImpl::RunScenario(const ScenarioInfo& scenario)
    {
        auto runScenario = [this, &scenario]
        {
            const auto scenarioScope = testExecution.StartScenario(scenario);

            currentScenario = &scenario;

            ExecuteSteps(scenario);
        };
        ASSERT_NO_THROW(runScenario());
    }

    void TestRunnerImpl::RunScenarios(const std::vector<std::unique_ptr<ScenarioInfo>>& scenarios)
    {
        for (const auto& scenario : scenarios)
            RunScenario(*scenario);
    }

    void TestRunnerImpl::ExecuteSteps(const ScenarioInfo& scenario)
    {
        for (const auto& step : scenario.Children())
            testExecution.RunStep(*step);
    }
}
