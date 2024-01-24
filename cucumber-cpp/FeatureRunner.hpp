#ifndef CUCUMBER_CPP_FEATURERUNNER_HPP
#define CUCUMBER_CPP_FEATURERUNNER_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/HookScopes.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include "cucumber/gherkin/app.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/pickle.hpp"
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>

namespace cucumber_cpp
{
    struct CucumberRunnerV2;

    struct FeatureSource
    {
        std::string name;
        std::filesystem::path path;
        std::size_t line;
        std::size_t column;

        static FeatureSource FromAst(const cucumber::gherkin::app::parser_result& ast);
    };

    struct FeatureRunnerV2
    {
    public:
        FeatureRunnerV2(CucumberRunnerV2& cucumberRunner, const cucumber::gherkin::app::parser_result& ast);
        ~FeatureRunnerV2();

        [[nodiscard]] const FeatureSource& Source() const;

        [[nodiscard]] report::ReportHandler& ReportHandler();
        [[nodiscard]] Context& GetContext();

        [[nodiscard]] const cucumber::messages::feature& Feature() const;

        [[nodiscard]] report::ReportHandler::Result Result() const;
        [[nodiscard]] TraceTime::Duration Duration() const;

        void StartScenario(const cucumber::messages::pickle& pickle);

    private:
        void StartFeatureOnce();
        void StopFeatureOnDestruction() noexcept;

        CucumberRunnerV2& cucumberRunner;
        const cucumber::gherkin::app::parser_result& ast;

        std::once_flag startFeatureOnceFlag;
        std::function<void()> stopFeatureOnDestruction{ [] {} };

        FeatureSource featureSource;

        report::ReportHandler::Result result{ report::ReportHandler::Result::undefined };
        TraceTime::Duration duration{ 0 };

        Context featureContext;
        BeforeAfterFeatureHookScope featureHookScope;
    };
}

#endif
