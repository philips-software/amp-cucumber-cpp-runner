#ifndef REPORT_JUNITREPORT_HPP
#define REPORT_JUNITREPORT_HPP

#include "cucumber-cpp/report/Report.hpp"
#include "pugixml.hpp"
#include <optional>
#include <tuple>
#include <vector>

namespace cucumber_cpp::report
{
    struct JunitReportV2 : ReportHandler
    {
        JunitReportV2();
        ~JunitReportV2();

        void FeatureStart(const FeatureSource& featureSource) override;
        void FeatureEnd(const FeatureSource& featureSource, Result result, TraceTime::Duration duration) override;

        void ScenarioStart(const ScenarioSource& scenarioSource) override;
        void ScenarioEnd(const ScenarioSource& scenarioSource, Result result, TraceTime::Duration duration) override;

        void StepStart(const StepSource& stepSource) override;
        void StepEnd(const StepSource& stepSource, Result result, TraceTime::Duration duration) override;

        void Failure(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column) override;
        void Error(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column) override;

        void Trace(const std::string& trace) override;

    private:
        pugi::xml_document doc;
        pugi::xml_node testsuites;
        pugi::xml_node testsuite;
        pugi::xml_node testcase;

        std::size_t totalTests{ 0 };
        std::size_t totalFailures{ 0 };
        std::size_t totalErrors{ 0 };
        std::size_t totalSkipped{ 0 };
        TraceTime::Duration totalTime{ 0 };

        std::size_t scenarioTests{ 0 };
        std::size_t scenarioFailures{ 0 };
        std::size_t scenarioErrors{ 0 };
        std::size_t scenarioSkipped{ 0 };

        //                 testsuite.append_attribute("tests").set_value(featureStatistics.tests);
        //                 testsuite.append_attribute("failures").set_value(featureStatistics.failures);
        //                 testsuite.append_attribute("errors").set_value(featureStatistics.errors);
        //                 testsuite.append_attribute("skipped").set_value(featureStatistics.skipped);

        // testsuites = doc.append_child("testsuites");
    };
}

#endif
