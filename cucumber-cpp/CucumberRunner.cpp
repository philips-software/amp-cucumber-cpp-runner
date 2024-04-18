#include "cucumber-cpp/CucumberRunner.hpp"
#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/FeatureRunner.hpp"
#include "cucumber-cpp/HookScopes.hpp"
#include "cucumber/gherkin/app.hpp"
#include <memory>
#include <string>
#include <utility>

namespace cucumber_cpp
{
    CucumberRunner::CucumberRunner(Context& programContext, std::string tagExpression, report::ReportHandler& reportHandler)
        : tagExpression{ std::move(tagExpression) }
        , reportHandler{ reportHandler }
        , programContext{ programContext }
        , programHookeScope{ programContext }
    {
    }

    std::string CucumberRunner::TagExpression() const
    {
        return tagExpression;
    }

    report::ReportHandler& CucumberRunner::ReportHandler()
    {
        return reportHandler;
    }

    Context& CucumberRunner::GetContext()
    {
        return programContext;
    }

    std::unique_ptr<FeatureRunnerV2> CucumberRunner::StartFeature(const cucumber::gherkin::app::parser_result& ast)
    {
        return std::make_unique<FeatureRunnerV2>(*this, ast);
    }
}
