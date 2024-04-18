
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

    FeatureRunner::FeatureRunner(CucumberRunner& cucumberRunner, const cucumber::gherkin::app::parser_result& ast)
        : cucumberRunner{ cucumberRunner }
        , ast{ ast }
        , featureSource{ FeatureSource::FromAst(ast) }
        , featureContext{ &cucumberRunner.GetContext() }
        , featureHookScope{ featureContext, TagsToSet(ast.feature->tags) }
    {
    }

    FeatureRunner::~FeatureRunner()
    {
        try
        {
            stopFeatureOnDestruction();
        }
        catch (...)
        {
            std::cout << "\nexception caught during FeatureRunner dtor";
        }
    }

    const FeatureSource& FeatureRunner::Source() const
    {
        return featureSource;
    }

    report::ReportHandler& FeatureRunner::ReportHandler()
    {
        return cucumberRunner.ReportHandler();
    }

    Context& FeatureRunner::GetContext()
    {
        return featureContext;
    }

    const cucumber::messages::feature& FeatureRunner::Feature() const
    {
        return ast.feature.value();
    }

    void FeatureRunner::StartScenario(const cucumber::messages::pickle& pickle)
    {
        if (ast.feature && IsTagExprSelected(cucumberRunner.TagExpression(), TagsToSet(pickle.tags)))
        {
            std::call_once(startFeatureOnceFlag, &FeatureRunner::StartFeatureOnce, *this);

            ScenarioRunner scenarioRunner{ *this, pickle };

            scenarioRunner.Run();

            duration += scenarioRunner.Duration();

            if (result == decltype(result)::undefined || result == decltype(result)::success)
                result = scenarioRunner.Result();
        }
    }

    report::ReportHandler::Result FeatureRunner::Result() const
    {
        return result;
    }

    TraceTime::Duration FeatureRunner::Duration() const
    {
        return duration;
    }

    void FeatureRunner::StartFeatureOnce()
    {
        ReportHandler().FeatureStart(Source());

        stopFeatureOnDestruction = [this]
        {
            StopFeatureOnDestruction();
        };
    }

    void FeatureRunner::StopFeatureOnDestruction() noexcept
    {
        ReportHandler().FeatureEnd(Source(), report::ReportHandler::Result::undefined, Duration());
    }
}
