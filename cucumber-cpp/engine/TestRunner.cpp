
#include "cucumber-cpp/engine/TestRunner.hpp"
#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "cucumber-cpp/engine/FeatureInfo.hpp"
#include "cucumber-cpp/engine/StepInfo.hpp"
#include "cucumber-cpp/engine/StepInfoBase.hpp"
#include "cucumber-cpp/report/Report.hpp"

namespace cucumber_cpp::engine
{
    [[nodiscard]] Result CurrentContext::ExecutionStatus() const
    {
        return executionStatus;
    }

    void CurrentContext::ExecutionStatus(Result result)
    {
        executionStatus = result;
    }

    namespace TestRunner
    {
        namespace
        {
            struct StepContext : RunnerContext
            {
                StepContext(RunnerContext& parent, const StepInfo& stepInfo, report::ReportHandlerV2& reportHandler)
                    : RunnerContext{ &parent }
                    , stepInfo{ stepInfo }
                    , reportHandler{ reportHandler }
                {
                    traceTime.Start();
                    reportHandler.StepStart(RunnerContext::ExecutionStatus(), stepInfo);
                }

                ~StepContext()
                {
                    traceTime.Stop();
                    reportHandler.StepEnd(RunnerContext::ExecutionStatus(), stepInfo, traceTime.Delta());
                }

                const StepInfo& stepInfo;
                report::ReportHandlerV2& reportHandler;

                TraceTime traceTime;
            };

            struct ScenarioContext : RunnerContext
            {
                using RunnerContext::RunnerContext;
            };

            struct FeatureContext : RunnerContext
            {
                using RunnerContext::RunnerContext;
            };

            [[nodiscard]] Result Run(RunnerContext& parent, const StepInfo& step, report::ReportHandlerV2& reportHandler)
            {
                StepContext stepContext{ parent, step, reportHandler };

                step.StepMatch().factory(parent, step.Table())->Execute(step.StepMatch().matches);

                return stepContext.ExecutionStatus();
            }

            [[nodiscard]] Result Run(RunnerContext& parent, const std::vector<StepInfo>& steps, report::ReportHandlerV2& reportHandler)
            {
                auto result = Result::passed;

                for (const auto& step : steps)
                {
                    auto stepResult = Run(parent, step, reportHandler);

                    if (result == Result::passed)
                        result = stepResult;
                }

                return result;
            }

            [[nodiscard]] Result Run(RunnerContext& parent, const ScenarioInfo& scenario, report::ReportHandlerV2& reportHandler)
            {
                ScenarioContext scenarioContext{ &parent };

                return Run(scenarioContext, scenario.Children(), reportHandler);
            }

            [[nodiscard]] Result Run(RunnerContext& parent, const std::vector<ScenarioInfo>& scenarios, report::ReportHandlerV2& reportHandler)
            {
                auto result = Result::passed;

                for (const auto& scenario : scenarios)
                {
                    auto scenarioResult = Run(parent, scenario, reportHandler);

                    if (result == Result::passed)
                        result = scenarioResult;
                }

                return result;
            }

            Result Run(RunnerContext& parent, const FeatureInfo& feature, report::ReportHandlerV2& reportHandler)
            {
                RunnerContext featureContext{ &parent };
                return Run(featureContext, feature.Children(), reportHandler);
            }
        }

        Result Run(RunnerContext& parent, const std::vector<FeatureInfo>& features, report::ReportHandlerV2& reportHandler)
        {
            auto result = Result::passed;

            for (const auto& feature : features)
            {
                auto featureResult = Run(parent, feature, reportHandler);

                if (result == Result::passed)
                    result = featureResult;
            }
            return result;
        }
    }
}
