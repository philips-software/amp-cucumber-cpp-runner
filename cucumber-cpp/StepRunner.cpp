#include "cucumber-cpp/StepRunner.hpp"
#include "cucumber-cpp/HookScopes.hpp"
#include "cucumber-cpp/InternalError.hpp"
#include "cucumber-cpp/OnTestPartResultEventListener.hpp"
#include "cucumber-cpp/Rtrim.hpp"
#include "cucumber-cpp/StepRegistry.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include "cucumber/messages/pickle_step_type.hpp"
#include "gtest/gtest.h"
#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace cucumber_cpp
{

    namespace
    {
        struct AppendFailureOnTestPartResultEvent
            : OnTestPartResultEventListener
        {
            explicit AppendFailureOnTestPartResultEvent(report::ReportHandler& reportHandler)
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

            void OnTestPartResult(const testing::TestPartResult& testPartResult) override
            {
                errors.emplace_back(testPartResult);
            }

            [[nodiscard]] bool HasFailures() const
            {
                return !errors.empty();
            }

        private:
            report::ReportHandler& reportHandler;
            std::vector<testing::TestPartResult> errors;
        };

        struct CaptureAndTraceStdOut
        {
            explicit CaptureAndTraceStdOut(report::ReportHandler& reportHandler)
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

        private:
            report::ReportHandler& reportHandler;
        };

        const std::map<cucumber::messages::pickle_step_type, StepType> stepTypeLut{
            { cucumber::messages::pickle_step_type::CONTEXT, StepType::given },
            { cucumber::messages::pickle_step_type::ACTION, StepType::when },
            { cucumber::messages::pickle_step_type::OUTCOME, StepType::then },
        };

        const std::map<cucumber::messages::pickle_step_type, std::string> stepTypeToString{
            { cucumber::messages::pickle_step_type::CONTEXT, "Given" },
            { cucumber::messages::pickle_step_type::ACTION, "When" },
            { cucumber::messages::pickle_step_type::OUTCOME, "Then" },
        };

        std::vector<std::vector<TableValue>> PickleArgumentToTable(const std::optional<cucumber::messages::pickle_step_argument>& optionalPickleStepArgument)
        {
            try
            {
                const auto& pickleStepArgument = optionalPickleStepArgument.value();
                const auto& optionalDataTable = pickleStepArgument.data_table;
                const auto& dataTable = optionalDataTable.value();

                std::vector<std::vector<TableValue>> table;

                for (const auto& row : dataTable.rows)
                {
                    table.emplace_back();

                    for (const auto& cols : row.cells)
                        table.back().emplace_back(cols.value);
                }
                return table;
            }
            catch (const std::bad_optional_access&)
            {
                return {};
            }
        }

        [[nodiscard]] StepSource LookupStepSource(const ScenarioSource& scenarioSource, const cucumber::messages::feature& feature, const cucumber::messages::pickle_step& pickleStep, const std::string& id)
        {
            for (const auto& child : feature.children)
                if (child.background)
                {
                    const auto iter = std::ranges::find(child.background->steps, id, &cucumber::messages::step::id);
                    if (iter != child.background->steps.end())
                        return StepSource::FromAst(scenarioSource, *iter, pickleStep);
                }
                else if (child.scenario)
                {
                    const auto iter = std::ranges::find(child.scenario->steps, id, &cucumber::messages::step::id);
                    if (iter != child.scenario->steps.end())
                        return StepSource::FromAst(scenarioSource, *iter, pickleStep);
                }

            struct StepSourceNotFoundError : std::out_of_range
            {
                using std::out_of_range::out_of_range;
            };

            throw StepSourceNotFoundError{ "StepSource not found" };
        }
    }

    StepSource StepSource::FromAst(const ScenarioSource& scenarioSource, const cucumber::messages::pickle_step& pickleStep)
    {
        return { scenarioSource, pickleStep.text, stepTypeToString.at(pickleStep.type.value()), 0, 0 };
    }

    StepSource StepSource::FromAst(const ScenarioSource& scenarioSource, const cucumber::messages::step& step, const cucumber::messages::pickle_step& pickleStep)
    {
        return { scenarioSource, pickleStep.text, stepTypeToString.at(pickleStep.type.value()), step.location.line, step.location.column.value_or(0) };
    }

    SkipStepRunnerV2::SkipStepRunnerV2(ScenarioRunner& scenarioRunner, const cucumber::messages::pickle_step& pickleStep)
        : scenarioRunner{ scenarioRunner }
        , stepSource{ LookupStepSource(scenarioRunner.Source(), scenarioRunner.Ast(), pickleStep, pickleStep.ast_node_ids[0]) }
    {
        scenarioRunner.ReportHandler().StepStart(stepSource);
    }

    SkipStepRunnerV2::~SkipStepRunnerV2()
    {
        scenarioRunner.ReportHandler().StepEnd(stepSource, Result(), Duration());
    }

    report::ReportHandler::Result SkipStepRunnerV2::Result() const
    {
        return report::ReportHandler::Result::skipped;
    }

    TraceTime::Duration SkipStepRunnerV2::Duration() const
    {
        return TraceTime::Duration{ 0 };
    }

    StepRunner::StepRunner(ScenarioRunner& scenarioRunner, const cucumber::messages::pickle_step& pickleStep)
        : scenarioRunner{ scenarioRunner }
        , pickleStep{ pickleStep }
        , stepSource{ LookupStepSource(scenarioRunner.Source(), scenarioRunner.Ast(), pickleStep, pickleStep.ast_node_ids[0]) }
    {
        ReportHandler().StepStart(Source());
    }

    StepRunner::~StepRunner()
    {
        ReportHandler().StepEnd(Source(), Result(), Duration());
    }

    const StepSource& StepRunner::Source() const
    {
        return stepSource;
    }

    report::ReportHandler& StepRunner::ReportHandler()
    {
        return scenarioRunner.ReportHandler();
    }

    report::ReportHandler::Result StepRunner::Result() const
    {
        return result;
    }

    TraceTime::Duration StepRunner::Duration() const
    {
        return traceTime.Delta();
    }

    void StepRunner::Run()
    {
        try
        {
            const auto stepMatch = StepRegistry::Instance().Query(stepTypeLut.at(*pickleStep.type), pickleStep.text);

            AppendFailureOnTestPartResultEvent appendFailureOnTestPartResultEvent{ ReportHandler() };
            CaptureAndTraceStdOut captureAndTraceStdOut{ ReportHandler() };
            TraceTime::Scoped scopedTime{ traceTime };

            BeforeAfterStepHookScope stepHookScope{ scenarioRunner.GetContext(), scenarioRunner.GetScenarioTags() };

            stepMatch.factory(scenarioRunner.GetContext(), PickleArgumentToTable(pickleStep.argument))->Execute(stepMatch.regexMatch->Matches());

            if (appendFailureOnTestPartResultEvent.HasFailures())
                result = decltype(result)::failed;
            else
                result = decltype(result)::success;
        }
        catch (const StepRegistryBase::StepNotFoundError& /* e */)
        {
            result = decltype(result)::error;
            ReportHandler().Error("Step \"" + stepSource.type + " " + stepSource.name + "\" not found");
        }
        catch (const StepRegistryBase::AmbiguousStepError& e)
        {
            std::ostringstream out;
            out << "Ambiguous step: " << pickleStep.text << "\nMatches:";

            for (const auto& step : e.matches)
                out << "\n\t" << step.stepRegex.String();

            result = decltype(result)::ambiguous;
            ReportHandler().Error(out.str());
        }
        catch (const Step::StepPending& e)
        {
            result = decltype(result)::pending;
            ReportHandler().Error(e.message, e.sourceLocation.file_name(), e.sourceLocation.line(), e.sourceLocation.column());
        }
        catch (const InternalError& e)
        {
            result = decltype(result)::error;
            ReportHandler().Error(e.what(), e.sourceLocation.file_name(), e.sourceLocation.line(), e.sourceLocation.column());
        }
        catch (const std::source_location& loc)
        {
            result = decltype(result)::error;
            ReportHandler().Error("Unknown error", loc.file_name(), loc.line(), loc.column());
        }
        catch (const std::exception& e)
        {
            result = decltype(result)::error;
            ReportHandler().Error(std::string{ "Exception thrown: " } + e.what());
        }
        catch (...)
        {
            result = decltype(result)::error;
            ReportHandler().Error("Unknown exception thrown");
        }
    }
}
