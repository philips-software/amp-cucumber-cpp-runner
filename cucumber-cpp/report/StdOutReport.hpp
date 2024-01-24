#ifndef REPORT_STDOUTREPORT_HPP
#define REPORT_STDOUTREPORT_HPP

#include "cucumber-cpp/report/Report.hpp"
#include <functional>
#include <optional>

namespace cucumber_cpp::report
{
    struct StdOutReportV2 : ReportHandler
    {
        void FeatureStart(const FeatureSource& featureSource) override;
        void FeatureEnd(const FeatureSource& featureSource, Result result, TraceTime::Duration duration) override;

        void ScenarioStart(const ScenarioSource& scenarioSource) override;
        void ScenarioEnd(const ScenarioSource& scenarioSource, Result result, TraceTime::Duration duration) override;

        void StepStart(const StepSource& stepSource) override;
        void StepEnd(const StepSource& stepSource, Result result, TraceTime::Duration duration) override;

        void Failure(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column) override;
        void Error(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column) override;

        void Trace(const std::string& trace) override;
    };
}

#endif
