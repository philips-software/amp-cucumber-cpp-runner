
#include "cucumber-cpp/engine/TestRunner.hpp"
#include "cucumber-cpp/HookRegistry.hpp"
#include "cucumber-cpp/OnTestPartResultEventListener.hpp"
#include "cucumber-cpp/Rtrim.hpp"
#include "cucumber-cpp/StepRegistry.hpp"
#include "cucumber-cpp/engine/ContextManager.hpp"
#include "cucumber-cpp/engine/FeatureInfo.hpp"
#include "cucumber-cpp/engine/Result.hpp"
#include "cucumber-cpp/engine/RuleInfo.hpp"
#include "cucumber-cpp/engine/StepInfo.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include "gtest/gtest.h"
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace cucumber_cpp::engine
{
    void RunTestPolicy::ExecuteStep(ContextManager& contextManager, const StepInfo& stepInfo, const StepMatch& stepMatch) const
    {
        stepMatch.factory(contextManager.ScenarioContext(), stepInfo.Table())->Execute(stepMatch.matches);
    }

    bool RunTestPolicy::ExecuteHook(ContextManager& context, HookType hook, const std::set<std::string, std::less<>>& tags) const
    {
        try
        {
            for (const auto& match : HookRegistry::Instance().Query(hook, tags))
                match.factory(context.CurrentContext())->Execute();
        }
        catch (std::exception& /* ex */)
        {
            return false;
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

    void DryRunPolicy::ExecuteStep(ContextManager& /* contextManager */, const StepInfo& /* stepInfo */, const StepMatch& /* stepMatch */) const
    {
    }

    bool DryRunPolicy::ExecuteHook(ContextManager& /* context */, HookType /* hook */, const std::set<std::string, std::less<>>& /* tags */) const
    {
        return true;
    }

    namespace TestRunner
    {
        namespace
        {
            struct AppendFailureOnTestPartResultEvent
                : OnTestPartResultEventListener
            {
                explicit AppendFailureOnTestPartResultEvent(report::ReportHandlerV2& reportHandler)
                    : reportHandler{ reportHandler }
                {
                }

                ~AppendFailureOnTestPartResultEvent() override
                {
                    for (const auto& error : errors)
                        reportHandler.Failure(error.message(), error.file_name(), error.line_number(), 0);
                }

                AppendFailureOnTestPartResultEvent(const AppendFailureOnTestPartResultEvent&) = delete;
                AppendFailureOnTestPartResultEvent& operator=(const AppendFailureOnTestPartResultEvent&) = delete;

                AppendFailureOnTestPartResultEvent(AppendFailureOnTestPartResultEvent&&) = delete;
                AppendFailureOnTestPartResultEvent& operator=(AppendFailureOnTestPartResultEvent&&) = delete;

                void OnTestPartResult(const testing::TestPartResult& testPartResult) override
                {
                    errors.emplace_back(testPartResult);
                }

                [[nodiscard]] bool HasFailures() const
                {
                    return !errors.empty();
                }

            private:
                report::ReportHandlerV2& reportHandler;
                std::vector<testing::TestPartResult> errors;
            };

            struct CaptureAndTraceStdOut
            {
                explicit CaptureAndTraceStdOut(report::ReportHandlerV2& reportHandler)
                    : reportHandler{ reportHandler }
                {
                    testing::internal::CaptureStdout();
                    testing::internal::CaptureStderr();
                }

                ~CaptureAndTraceStdOut()
                {
                    if (auto str = testing::internal::GetCapturedStdout(); !str.empty())
                    {
                        Rtrim(str);
                        reportHandler.Trace(str);
                    }

                    if (auto str = testing::internal::GetCapturedStderr(); !str.empty())
                    {
                        Rtrim(str);
                        reportHandler.Trace(str);
                    }
                }

                CaptureAndTraceStdOut(const CaptureAndTraceStdOut&) = delete;
                CaptureAndTraceStdOut& operator=(const CaptureAndTraceStdOut&) = delete;

                CaptureAndTraceStdOut(CaptureAndTraceStdOut&&) = delete;
                CaptureAndTraceStdOut& operator=(CaptureAndTraceStdOut&&) = delete;

            private:
                report::ReportHandlerV2& reportHandler;
            };

            struct ExecuteHookOnDescruction
            {
                ExecuteHookOnDescruction(ContextManager& context, HookType hook, const std::set<std::string, std::less<>>& tags, const RunPolicy& runPolicy)
                    : context{ context }
                    , hook{ hook }
                    , tags{ tags }
                    , runPolicy{ runPolicy }
                {}

                ~ExecuteHookOnDescruction()
                {
                    runPolicy.ExecuteHook(context, hook, tags);
                }

                ExecuteHookOnDescruction(const ExecuteHookOnDescruction&) = delete;
                ExecuteHookOnDescruction(ExecuteHookOnDescruction&&) = delete;

            private:
                ContextManager& context;
                HookType hook;
                const std::set<std::string, std::less<>>& tags;
                const RunPolicy& runPolicy;
            };

            [[noreturn]] void ExecuteStep(const ContextManager& /* contextManager */, const StepInfo& /* stepInfo */, std::monostate /* */, const RunPolicy& /* runPolicy */)
            {
                throw StepNotFoundException{};
            }

            [[noreturn]] void ExecuteStep(const ContextManager& /* contextManager */, const StepInfo& /* stepInfo */, const std::vector<StepMatch>& /* */, const RunPolicy& /* runPolicy */)
            {
                throw AmbiguousStepException{};
            }

            void ExecuteStep(ContextManager& contextManager, const StepInfo& stepInfo, const StepMatch& stepMatch, const RunPolicy& runPolicy)
            {
                ExecuteHookOnDescruction afterStepHook{ contextManager, HookType::afterStep, contextManager.ScenarioContext().scenarioInfo.Tags(), runPolicy };

                if (runPolicy.ExecuteHook(contextManager, HookType::beforeStep, contextManager.ScenarioContext().scenarioInfo.Tags()))
                    runPolicy.ExecuteStep(contextManager, stepInfo, stepMatch);
            }

            void StepResult(ContextManager& contextManager, Result result)
            {
                contextManager.CurrentContext().ExecutionStatus(result);
            }

            void ExecuteStep(ContextManager& contextManager, const StepInfo& stepInfo, const RunPolicy& runPolicy)
            {
                try
                {
                    std::visit([&contextManager, &stepInfo, &runPolicy](auto& value)
                        {
                            ExecuteStep(contextManager, stepInfo, value, runPolicy);
                        },
                        stepInfo.StepMatch());
                }
                catch (PendingException& /* ex */)
                {
                    StepResult(contextManager, Result::pending);
                }
                catch (StepNotFoundException& /* ex */)
                {
                    StepResult(contextManager, Result::undefined);
                }
                catch (AmbiguousStepException& /* ex */)
                {
                    StepResult(contextManager, Result::ambiguous);
                }
                catch (std::exception& /* ex */) // NOLINT
                {
                    StepResult(contextManager, Result::failed);
                }
                catch (...)
                {
                    StepResult(contextManager, Result::failed);
                }
            }
        }

        void WrapExecuteStep(ContextManager& contextManager, const StepInfo& step, report::ReportHandlerV2& reportHandler, const RunPolicy& runPolicy)
        {
            AppendFailureOnTestPartResultEvent appendFailureOnTestPartResultEvent{ reportHandler };
            CaptureAndTraceStdOut captureAndTraceStdOut{ reportHandler };

            ExecuteStep(contextManager, step, runPolicy);

            if (appendFailureOnTestPartResultEvent.HasFailures())
                StepResult(contextManager, engine::Result::failed);
        }

        void ManageExecuteStep(ContextManager& contextManager, const StepInfo& step, report::ReportHandlerV2& reportHandler, const RunPolicy& runPolicy)
        {
            contextManager.StartStep(step);

            if (const auto mustSkip = contextManager.ScenarioContext().ExecutionStatus() != Result::passed)
                reportHandler.StepSkipped(step);
            else
            {
                reportHandler.StepStart(step);

                WrapExecuteStep(contextManager, step, reportHandler, runPolicy);

                reportHandler.StepEnd(contextManager.StepContext().ExecutionStatus(), step, contextManager.StepContext().Duration());
            }

            contextManager.StopStep();
        }

        void ExecuteSteps(ContextManager& contextManager, const ScenarioInfo& scenario, report::ReportHandlerV2& reportHandler, const RunPolicy& runPolicy)
        {
            for (const auto& step : scenario.Children())
            {
                ManageExecuteStep(contextManager, *step, reportHandler, runPolicy);
            }
        }

        void RunScenarios(ContextManager& contextManager, const std::vector<std::unique_ptr<ScenarioInfo>>& scenarios, report::ReportHandlerV2& reportHandler, const RunPolicy& runPolicy)
        {
            for (const auto& scenario : scenarios)
            {
                contextManager.StartScenario(*scenario);

                reportHandler.ScenarioStart(*scenario);

                if (runPolicy.ExecuteHook(contextManager, HookType::before, scenario->Tags()))
                    ExecuteSteps(contextManager, *scenario, reportHandler, runPolicy);

                runPolicy.ExecuteHook(contextManager, HookType::after, scenario->Tags());

                reportHandler.ScenarioEnd(contextManager.ScenarioContext().ExecutionStatus(), *scenario, contextManager.ScenarioContext().Duration());

                contextManager.StopScenario();
            }
        }

        void RunRules(ContextManager& contextManager, const std::vector<std::unique_ptr<RuleInfo>>& rules, report::ReportHandlerV2& reportHandler, const RunPolicy& runPolicy)
        {
            for (const auto& rule : rules)
            {
                contextManager.StartRule(*rule);

                reportHandler.RuleStart(*rule);

                RunScenarios(contextManager, rule->Scenarios(), reportHandler, runPolicy);

                reportHandler.RuleEnd(contextManager.RuleContext().ExecutionStatus(), *rule, contextManager.RuleContext().Duration());

                contextManager.StopRule();
            }
        }

        void RunFeature(ContextManager& contextManager, const FeatureInfo& feature, report::ReportHandlerV2& reportHandler, const RunPolicy& runPolicy)
        {
            if (feature.Rules().empty() && feature.Scenarios().empty())
                return;

            contextManager.StartFeature(feature);

            reportHandler.FeatureStart(feature);

            if (runPolicy.ExecuteHook(contextManager, HookType::beforeFeature, feature.Tags()))
            {
                RunRules(contextManager, feature.Rules(), reportHandler, runPolicy);
                RunScenarios(contextManager, feature.Scenarios(), reportHandler, runPolicy);
            }

            runPolicy.ExecuteHook(contextManager, HookType::afterFeature, feature.Tags());

            reportHandler.FeatureEnd(contextManager.FeatureContext().ExecutionStatus(), feature, contextManager.FeatureContext().Duration());

            contextManager.StopFeature();
        }

        void Run(ContextManager& contextManager, const std::vector<std::unique_ptr<FeatureInfo>>& feature, report::ReportHandlerV2& reportHandler, const RunPolicy& runPolicy)
        {
            if (runPolicy.ExecuteHook(contextManager, HookType::beforeAll, {}))
                for (const auto& featurePtr : feature)
                    RunFeature(contextManager, *featurePtr, reportHandler, runPolicy);

            runPolicy.ExecuteHook(contextManager, HookType::afterAll, {});

            reportHandler.Summary(contextManager.ProgramContext().Duration());
        }
    }
}