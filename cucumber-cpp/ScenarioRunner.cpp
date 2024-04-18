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

            struct ScenarioSourceNotFoundError : std::out_of_range
            {
                using std::out_of_range::out_of_range;
            };

            throw ScenarioSourceNotFoundError{ "ScenarioSource not found" };
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

    ScenarioRunner::ScenarioRunner(FeatureRunner& featureRunner, const cucumber::messages::pickle& scenarioPickle)
        : featureRunner{ featureRunner }
        , pickle{ scenarioPickle }
        , scenarioTags{ TagsToSet(scenarioPickle.tags) }
        , scenarioSource{ LookupScenarioSource(featureRunner.Source(), Ast(), pickle.ast_node_ids[0]) }
        , scenarioContext{ &featureRunner.GetContext() }
        , scenarioHookScope{ scenarioContext, scenarioTags }
    {
        ReportHandler().ScenarioStart(scenarioSource);
    }

    ScenarioRunner::~ScenarioRunner()
    {
        ReportHandler().ScenarioEnd(scenarioSource, Result(), Duration());
    }

    const ScenarioSource& ScenarioRunner::Source() const
    {
        return scenarioSource;
    }

    report::ReportHandler& ScenarioRunner::ReportHandler()
    {
        return featureRunner.ReportHandler();
    }

    Context& ScenarioRunner::GetContext()
    {
        return scenarioContext;
    }

    const std::set<std::string, std::less<>>& ScenarioRunner::GetScenarioTags() const
    {
        return scenarioTags;
    }

    const cucumber::messages::feature& ScenarioRunner::Ast() const
    {
        return featureRunner.Feature();
    }

    report::ReportHandler::Result ScenarioRunner::Result() const
    {
        return result;
    }

    TraceTime::Duration ScenarioRunner::Duration() const
    {
        return duration;
    }

    void ScenarioRunner::Run()
    {
        for (const auto& step : pickle.steps)
            if (result == decltype(result)::success || result == decltype(result)::undefined)
            {
                StepRunner stepRunner{ *this, step };
                stepRunner.Run();

                duration += stepRunner.Duration();

                if (const auto stepResult = stepRunner.Result(); stepResult != decltype(stepResult)::success || stepResult != decltype(stepResult)::skipped)
                    result = stepResult;
            }
            else
                SkipStepRunner{ *this, step };
    }
}
