
#include "cucumber-cpp/FeatureRunner.hpp"
#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/CucumberRunner.hpp"
#include "cucumber-cpp/ScenarioRunner.hpp"
#include "cucumber-cpp/TagExpression.hpp"
#include "cucumber-cpp/TagsToSet.hpp"
#include "cucumber/gherkin/app.hpp"
#include "cucumber/messages/feature.hpp"
#include <iostream>
#include <memory>
#include <mutex>
#include <type_traits>

namespace cucumber_cpp
{
    FeatureSource FeatureSource::FromAst(const cucumber::gherkin::app::parser_result& ast)
    {
        return { ast.feature->name, ast.uri.value_or("unknown"), ast.feature->location.line, ast.feature->location.column.value_or(0) };
    }

    FeatureRunnerV2::FeatureRunnerV2(CucumberRunnerV2& cucumberRunner, const cucumber::gherkin::app::parser_result& ast)
        : cucumberRunner{ cucumberRunner }
        , ast{ ast }
        , featureSource{ FeatureSource::FromAst(ast) }
        , featureContext{ &cucumberRunner.GetContext() }
        , featureHookScope{ featureContext, TagsToSet(ast.feature->tags) }
    {
    }

    FeatureRunnerV2::~FeatureRunnerV2()
    {
        stopFeatureOnDestruction();
    }

    const FeatureSource& FeatureRunnerV2::Source() const
    {
        return featureSource;
    }

    report::ReportHandler& FeatureRunnerV2::ReportHandler()
    {
        return cucumberRunner.ReportHandler();
    }

    Context& FeatureRunnerV2::GetContext()
    {
        return featureContext;
    }

    const cucumber::messages::feature& FeatureRunnerV2::Feature() const
    {
        return ast.feature.value();
    }

    void FeatureRunnerV2::StartScenario(const cucumber::messages::pickle& pickle)
    {
        if (ast.feature && IsTagExprSelected(cucumberRunner.TagExpression(), TagsToSet(pickle.tags)))
        {
            std::call_once(startFeatureOnceFlag, &FeatureRunnerV2::StartFeatureOnce, *this);

            ScenarioRunnerV2 scenarioRunner{ *this, pickle };

            scenarioRunner.Run();

            duration += scenarioRunner.Duration();

            if (result == decltype(result)::undefined || result == decltype(result)::success)
                result = scenarioRunner.Result();
        }
    }

    report::ReportHandler::Result FeatureRunnerV2::Result() const
    {
        return result;
    }

    TraceTime::Duration FeatureRunnerV2::Duration() const
    {
        return duration;
    }

    void FeatureRunnerV2::StartFeatureOnce()
    {
        ReportHandler().FeatureStart(Source());

        stopFeatureOnDestruction = [this]
        {
            StopFeatureOnDestruction();
        };
    }

    void FeatureRunnerV2::StopFeatureOnDestruction()
    {
        ReportHandler().FeatureEnd(Source(), report::ReportHandler::Result::undefined, Duration());
    }
}
