#ifndef CUCUMBER_CPP_CUCUMBERRUNNER_HPP
#define CUCUMBER_CPP_CUCUMBERRUNNER_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/HookScopes.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include "cucumber/gherkin/app.hpp"
#include <memory>
#include <string>

namespace cucumber_cpp
{
    struct FeatureRunnerV2;

    struct CucumberRunnerV2
    {
    public:
        CucumberRunnerV2(Context& programContext, std::string tagExpression, report::ReportHandler& reportHandler);
        ~CucumberRunnerV2() = default;

        [[nodiscard]] std::string TagExpression() const;
        [[nodiscard]] report::ReportHandler& ReportHandler();
        [[nodiscard]] Context& GetContext();

        std::unique_ptr<FeatureRunnerV2> StartFeature(const cucumber::gherkin::app::parser_result& ast);

    private:
        const std::string tagExpression;
        report::ReportHandler& reportHandler;

        Context& programContext;
        BeforeAfterAllScope programHookeScope;
    };
}

#endif
