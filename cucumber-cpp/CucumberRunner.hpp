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
    struct FeatureRunner;

    struct CucumberRunner
    {
    public:
        CucumberRunner(Context& programContext, std::string tagExpression, report::ReportHandler& reportHandler);
        ~CucumberRunner() = default;

        [[nodiscard]] std::string TagExpression() const;
        [[nodiscard]] report::ReportHandler& ReportHandler();
        [[nodiscard]] Context& GetContext();

        std::unique_ptr<FeatureRunner> StartFeature(const cucumber::gherkin::app::parser_result& ast);

    private:
        const std::string tagExpression;
        report::ReportHandler& reportHandler;

        Context& programContext;
        BeforeAfterAllScope programHookeScope;
    };
}

#endif
