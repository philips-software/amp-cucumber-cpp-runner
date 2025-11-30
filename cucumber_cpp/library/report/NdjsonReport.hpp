#pragma once
#include "Report.hpp"
#include <fstream>

namespace cucumber_cpp::library::report
{
    struct NdjsonReport : ReportHandlerV2
    {
        NdjsonReport(const std::string& outputFolder, const std::string& reportFile);
        ~NdjsonReport() override;

        void FeatureStart(const engine::FeatureInfo& featureInfo) override;
        void FeatureEnd(engine::Result result, const engine::FeatureInfo& featureInfo, TraceTime::Duration duration) override;
        void RuleStart(const engine::RuleInfo& ruleInfo) override;
        void RuleEnd(engine::Result result, const engine::RuleInfo& ruleInfo, TraceTime::Duration duration) override;
        void ScenarioStart(const engine::ScenarioInfo& scenarioInfo) override;
        void ScenarioEnd(engine::Result result, const engine::ScenarioInfo& scenarioInfo, TraceTime::Duration duration) override;
        void StepSkipped(const engine::StepInfo& stepInfo) override;
        void StepStart(const engine::StepInfo& stepInfo) override;
        void StepEnd(engine::Result result, const engine::StepInfo& stepInfo, TraceTime::Duration duration) override;
        void Failure(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column) override;
        void Error(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column) override;
        void Trace(const std::string& trace) override;
        void Summary(TraceTime::Duration duration) override;

    private:
        const std::string& outputFolder;
        const std::string& reportFile;

        std::ofstream outStream;
        void InitReportDirectory();
        void RecordMeta() ;

    };

}
