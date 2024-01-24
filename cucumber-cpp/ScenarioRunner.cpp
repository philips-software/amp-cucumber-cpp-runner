#include "cucumber-cpp/ScenarioRunner.hpp"
#include "cucumber-cpp/FeatureRunner.hpp"
#include "cucumber-cpp/HookScopes.hpp"
#include "cucumber-cpp/StepRunner.hpp"
#include "cucumber-cpp/TagsToSet.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include <algorithm>
#include <cstddef>
#include <memory>

namespace cucumber_cpp
{
    namespace
    {
        [[nodiscard]] ScenarioSource LookupScenarioSource(const FeatureSource& featureSource, const cucumber::messages::feature& feature, const std::string& id)
        {
            for (const auto& child : feature.children)
                if (child.background && child.background->id == id)
                    return ScenarioSource::FromAst(featureSource, *child.background);
                else if (child.scenario && child.scenario->id == id)
                    return ScenarioSource::FromAst(featureSource, *child.scenario);

            throw std::runtime_error{ "ScenarioSource not found" };
        }
    }

    ScenarioSource ScenarioSource::FromAst(const FeatureSource& featureSource, const cucumber::messages::scenario& scenario)
    {
        return { featureSource, scenario.name, scenario.location.line, scenario.location.column.value_or(0) };
    }

    ScenarioSource ScenarioSource::FromAst(const FeatureSource& featureSource, const cucumber::messages::background& background)
    {
        return { featureSource, background.name, background.location.line, background.location.column.value_or(0) };
    }

    ScenarioRunnerV2::ScenarioRunnerV2(FeatureRunnerV2& featureRunner, const cucumber::messages::pickle& scenarioPickle)
        : featureRunner{ featureRunner }
        , pickle{ scenarioPickle }
        , scenarioTags{ TagsToSet(scenarioPickle.tags) }
        , scenarioSource{ LookupScenarioSource(featureRunner.Source(), Ast(), pickle.ast_node_ids[0]) }
        , scenarioContext{ &featureRunner.GetContext() }
        , scenarioHookScope{ scenarioContext, scenarioTags }
    {
        ReportHandler().ScenarioStart(scenarioSource);
    }

    ScenarioRunnerV2::~ScenarioRunnerV2()
    {
        ReportHandler().ScenarioEnd(scenarioSource, Result(), Duration());
    }

    const ScenarioSource& ScenarioRunnerV2::Source() const
    {
        return scenarioSource;
    }

    report::ReportHandler& ScenarioRunnerV2::ReportHandler()
    {
        return featureRunner.ReportHandler();
    }

    Context& ScenarioRunnerV2::GetContext()
    {
        return scenarioContext;
    }

    const std::set<std::string>& ScenarioRunnerV2::GetScenarioTags() const
    {
        return scenarioTags;
    }

    const cucumber::messages::feature& ScenarioRunnerV2::Ast() const
    {
        return featureRunner.Feature();
    }

    report::ReportHandler::Result ScenarioRunnerV2::Result() const
    {
        return result;
    }

    TraceTime::Duration ScenarioRunnerV2::Duration() const
    {
        return duration;
    }

    void ScenarioRunnerV2::Run()
    {
        for (const auto& step : pickle.steps)
            if (result == decltype(result)::success || result == decltype(result)::undefined)
            {
                StepRunnerV2 stepRunner{ *this, step };
                stepRunner.Run();

                duration += stepRunner.Duration();

                if (const auto result = stepRunner.Result(); result != decltype(result)::success || result != decltype(result)::skipped)
                    this->result = result;
            }
            else
                SkipStepRunnerV2{ *this, step };
    }
}
