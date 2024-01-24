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
    CucumberRunnerV2::InsertArgsToContext::InsertArgsToContext(Context& context, const std::vector<std::string_view>& args)
    {

        context.InsertAt("args", args);
    }

    CucumberRunnerV2::CucumberRunnerV2(const std::vector<std::string_view>& args, std::string tagExpression, report::ReportHandler& reportHandler, std::shared_ptr<ContextStorageFactory> contextStorageFactory)
        : tagExpression{ std::move(tagExpression) }
        , reportHandler{ reportHandler }
        , programContext{ std::move(contextStorageFactory) }
        , insertArgsToContext{ programContext, args }
        , programHookeScope{ programContext }
    {
    }

    std::string CucumberRunnerV2::TagExpression() const
    {
        return tagExpression;
    }

    report::ReportHandler& CucumberRunnerV2::ReportHandler()
    {
        return reportHandler;
    }

    Context& CucumberRunnerV2::GetContext()
    {
        return programContext;
    }

    std::unique_ptr<FeatureRunnerV2> CucumberRunnerV2::StartFeature(const cucumber::gherkin::app::parser_result& ast)
    {
        return std::make_unique<FeatureRunnerV2>(*this, ast);
    }
}
