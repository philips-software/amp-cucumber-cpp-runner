#ifndef REPORT_JUNITREPORT_HPP
#define REPORT_JUNITREPORT_HPP

#include "cucumber-cpp/report/Report.hpp"
#include "pugixml.hpp"
#include <optional>
#include <tuple>
#include <vector>

namespace cucumber_cpp::report
{
    struct JunitReport : ReportHandlerV2
    {
        JunitReport(const std::string& outputfolder, const std::string& reportfile);
        ~JunitReport() override;

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

    private:
        const std::string& outputfolder;
        const std::string& reportfile;

        pugi::xml_document doc;
        pugi::xml_node testsuites;
        pugi::xml_node testsuite;
        pugi::xml_node testcase;

        std::size_t totalTests{ 0 };
        std::size_t totalFailures{ 0 };
        std::size_t totalSkipped{ 0 };
        TraceTime::Duration totalTime{ 0 };

        std::size_t scenarioTests{ 0 };
        std::size_t scenarioFailures{ 0 };
        std::size_t scenarioSkipped{ 0 };
    };
}

#endif
