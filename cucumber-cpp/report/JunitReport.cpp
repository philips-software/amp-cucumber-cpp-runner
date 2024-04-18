
#include "cucumber-cpp/report/JunitReport.hpp"
#include "cucumber-cpp/FeatureRunner.hpp"
#include "cucumber-cpp/ScenarioRunner.hpp"
#include "cucumber-cpp/StepRunner.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <map>
#include <ranges>
#include <sstream>

namespace cucumber_cpp::report
{
    namespace
    {
        constexpr double precision = 0.0000001;

        const std::map<report::ReportHandler::Result, std::string> successLut{
            { report::ReportHandler::Result::success, "done" },
            { report::ReportHandler::Result::skipped, "skipped" },
            { report::ReportHandler::Result::failed, "failed" },
            { report::ReportHandler::Result::error, "error" },
            { report::ReportHandler::Result::pending, "pending" },
            { report::ReportHandler::Result::ambiguous, "ambiguous" },
            { report::ReportHandler::Result::undefined, "undefined" },
        };

        std::string RoundTo(double value, double roundToPrecision)
        {
            const auto d = std::round(value / roundToPrecision) * roundToPrecision;

            std::ostringstream out;
            out << std::fixed << d;
            return out.str();
        }
    }

    JunitReport::JunitReport(const std::string& outputfolder, const std::string& reportfile)
        : outputfolder{ outputfolder }
        , reportfile{ reportfile }
    {
        testsuites = doc.append_child("testsuites");
        testsuites.append_attribute("name").set_value("Test run");
        testsuites.append_attribute("time").set_value(0.0);
    }

    JunitReport::~JunitReport()
    {
        testsuites.append_attribute("tests").set_value(totalTests);
        testsuites.append_attribute("failures").set_value(totalFailures);
        testsuites.append_attribute("errors").set_value(totalErrors);
        testsuites.append_attribute("skipped").set_value(totalSkipped);

        const auto doubleTime = std::chrono::duration<double, std::ratio<1>>(totalTime).count();
        testsuites.append_attribute("time").set_value(RoundTo(doubleTime, precision).c_str());

        if (!std::filesystem::exists(outputfolder))
            std::filesystem::create_directories(outputfolder);
        const auto outputfile = std::filesystem::path{ outputfolder }.append(reportfile + ".xml");
        doc.save_file(outputfile.c_str());
    }

    void JunitReport::FeatureStart(const FeatureSource& featureSource)
    {
        testsuite = testsuites.append_child("testsuite");
        testsuite.append_attribute("name").set_value(featureSource.name.c_str());
        testsuite.append_attribute("file").set_value(featureSource.path.string().c_str());

        scenarioTests = 0;
        scenarioFailures = 0;
        scenarioErrors = 0;
        scenarioSkipped = 0;
    }

    void JunitReport::FeatureEnd(const FeatureSource& /*featureSource*/, Result /*result*/, TraceTime::Duration duration)
    {
        const auto doubleTime = std::chrono::duration<double, std::ratio<1>>(duration).count();
        testsuite.append_attribute("time").set_value(RoundTo(doubleTime, precision).c_str());

        totalTests += scenarioTests;
        totalFailures += scenarioFailures;
        totalErrors += scenarioErrors;
        totalSkipped += scenarioSkipped;

        testsuite.append_attribute("tests").set_value(scenarioTests);
        testsuite.append_attribute("failures").set_value(scenarioFailures);
        testsuite.append_attribute("errors").set_value(scenarioErrors);
        testsuite.append_attribute("skipped").set_value(scenarioSkipped);
    }

    void JunitReport::ScenarioStart(const ScenarioSource& scenarioSource)
    {
        testcase = testsuite.append_child("testcase");

        testcase.append_attribute("name").set_value(scenarioSource.name.c_str());

        ++scenarioTests;
    }

    void JunitReport::ScenarioEnd(const ScenarioSource& /*scenarioSource*/, Result result, TraceTime::Duration duration)
    {
        const auto doubleTime = std::chrono::duration<double, std::ratio<1>>(duration).count();
        testcase.append_attribute("time").set_value(RoundTo(doubleTime, precision).c_str());

        switch (result)
        {
            case ReportHandler::Result::skipped:
            case ReportHandler::Result::pending:
            case ReportHandler::Result::ambiguous:
            case ReportHandler::Result::undefined:
            {
                ++scenarioSkipped;
                auto skipped = testcase.append_child("skipped");

                if (result == ReportHandler::Result::skipped)
                {
                    skipped.append_attribute("message").set_value("Test is skipped due to previous errors.");
                }
                else if (result == ReportHandler::Result::undefined)
                {
                    skipped.append_attribute("message").set_value("Test is undefined.");
                }
                else if (result == ReportHandler::Result::pending)
                {
                    skipped.append_attribute("message").set_value("Test is pending.");
                }
                else
                {
                    skipped.append_attribute("message").set_value("Test result unkown.");
                }
            }

            break;

            case ReportHandler::Result::failed:
                ++scenarioFailures;
                break;

            case ReportHandler::Result::error:
                ++scenarioErrors;
                break;

            default:
                break;
        }

        totalTime += duration;
    }

    void JunitReport::StepStart(const StepSource& stepSource)
    {
        /* do nothing */
    }

    void JunitReport::StepEnd(const StepSource& stepSource, Result result, TraceTime::Duration duration)
    {
        /* do nothing */
    }

    void JunitReport::Failure(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
        auto failure = testcase.append_child("failure");

        failure.append_attribute("message").set_value(error.c_str());

        std::ostringstream out;

        if (path && line && column)
            out
                << "\n"
                << path.value().string() << ":" << line.value() << ":" << column.value() << ": Failure\n"
                << error;
        else
            out
                << "\n"
                << error;

        failure.text() = out.str().c_str();
    }

    void JunitReport::Error(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
        auto errorNode = testcase.append_child("error");

        errorNode.append_attribute("message").set_value(error.c_str());

        std::ostringstream out;

        if (path && line && column)
            out
                << "\n"
                << path.value().string() << ":" << line.value() << ":" << column.value() << ": Error\n"
                << error;
        else
            out
                << "\n"
                << error;

        errorNode.text() = out.str().c_str();
    }

    void JunitReport::Trace(const std::string& trace)
    {
        /* do nothing */
    }
}
