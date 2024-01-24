#ifndef CUCUMBER_CPP_SCENARIORUNNER_HPP
#define CUCUMBER_CPP_SCENARIORUNNER_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/HookScopes.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include "cucumber/messages/background.hpp"
#include "cucumber/messages/feature.hpp"
#include "cucumber/messages/pickle.hpp"
#include "cucumber/messages/scenario.hpp"
#include <memory>

namespace cucumber_cpp
{
    struct FeatureSource;

    struct ScenarioSource
    {
        const FeatureSource& featureSource;
        const std::string name;
        const std::size_t line;
        const std::size_t column;

        static ScenarioSource FromAst(const FeatureSource& featureSource, const cucumber::messages::scenario& scenario);
        static ScenarioSource FromAst(const FeatureSource& featureSource, const cucumber::messages::background& background);
    };

    struct FeatureRunnerV2;

    struct ScenarioRunnerV2
    {
    public:
        ScenarioRunnerV2(FeatureRunnerV2& featureRunner, const cucumber::messages::pickle& pickle);
        ~ScenarioRunnerV2();

        [[nodiscard]] const ScenarioSource& Source() const;

        [[nodiscard]] report::ReportHandler& ReportHandler();
        [[nodiscard]] Context& GetContext();
        [[nodiscard]] const std::set<std::string, std::less<>>& GetScenarioTags() const;

        [[nodiscard]] const cucumber::messages::feature& Ast() const;

        [[nodiscard]] report::ReportHandler::Result Result() const;
        [[nodiscard]] TraceTime::Duration Duration() const;

        void Run();

    private:
        FeatureRunnerV2& featureRunner;
        const cucumber::messages::pickle& pickle;

        std::set<std::string, std::less<>> scenarioTags;

        report::ReportHandler::Result result{ report::ReportHandler::Result::undefined };
        TraceTime::Duration duration{ 0 };

        ScenarioSource scenarioSource;

        Context scenarioContext;
        BeforeAfterHookScope scenarioHookScope;
    };
}

#endif
