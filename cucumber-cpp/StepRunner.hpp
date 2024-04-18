#ifndef CUCUMBER_CPP_STEPRUNNER_HPP
#define CUCUMBER_CPP_STEPRUNNER_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/Hooks.hpp"
#include "cucumber-cpp/ScenarioRunner.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "cucumber/messages/pickle_step.hpp"
#include <memory>

namespace cucumber_cpp
{
    struct StepSource
    {
        const ScenarioSource& scenarioSource;
        const std::string name;
        const std::string type;
        const std::size_t line;
        const std::size_t column;

        static StepSource FromAst(const ScenarioSource& scenarioSource, const cucumber::messages::pickle_step& pickleStep);
        static StepSource FromAst(const ScenarioSource& scenarioSource, const cucumber::messages::step& step, const cucumber::messages::pickle_step& pickleStep);
    };

    struct SkipStepRunner
    {
        SkipStepRunner(ScenarioRunner& scenarioRunner, const cucumber::messages::pickle_step& pickleStep);
        virtual ~SkipStepRunner();

        [[nodiscard]] report::ReportHandler::Result Result() const;
        [[nodiscard]] TraceTime::Duration Duration() const;

    private:
        ScenarioRunner& scenarioRunner;
        StepSource stepSource;
    };

    struct StepRunner
    {
    public:
        StepRunner(ScenarioRunner& scenarioRunner, const cucumber::messages::pickle_step& pickleStep);
        ~StepRunner();

        [[nodiscard]] const StepSource& Source() const;

        [[nodiscard]] report::ReportHandler& ReportHandler();

        [[nodiscard]] report::ReportHandler::Result Result() const;
        [[nodiscard]] TraceTime::Duration Duration() const;

        void Run();

    private:
        ScenarioRunner& scenarioRunner;
        const cucumber::messages::pickle_step& pickleStep;

        TraceTime traceTime;

        report::ReportHandler::Result result{ report::ReportHandler::Result::undefined };

        StepSource stepSource;
    };
}

#endif
