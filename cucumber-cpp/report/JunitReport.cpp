
#include "cucumber-cpp/report/JunitReport.hpp"
#include "cucumber-cpp/engine/Result.hpp"
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

        const std::map<engine::Result, std::string> successLut{
            { engine::Result::passed, "done" },
            { engine::Result::skipped, "skipped" },
            { engine::Result::failed, "failed" },
            { engine::Result::pending, "pending" },
            { engine::Result::ambiguous, "ambiguous" },
            { engine::Result::undefined, "undefined" },
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
        testsuites.append_attribute("skipped").set_value(totalSkipped);

        const auto doubleTime = std::chrono::duration<double, std::ratio<1>>(totalTime).count();
        testsuites.append_attribute("time").set_value(RoundTo(doubleTime, precision).c_str());

        if (!std::filesystem::exists(outputfolder))
            std::filesystem::create_directories(outputfolder);
        const auto outputfile = std::filesystem::path{ outputfolder }.append(reportfile + ".xml");
        doc.save_file(outputfile.c_str());
    }

    void JunitReport::FeatureStart(const engine::FeatureInfo& featureInfo)
    {
        testsuite = testsuites.append_child("testsuite");
        testsuite.append_attribute("name").set_value(featureInfo.Title().c_str());
        testsuite.append_attribute("file").set_value(featureInfo.Path().string().c_str());

        scenarioTests = 0;
        scenarioFailures = 0;
        scenarioSkipped = 0;
    }

    void JunitReport::FeatureEnd(engine::Result result, const engine::FeatureInfo& featureInfo, TraceTime::Duration duration)
    {
        const auto doubleTime = std::chrono::duration<double, std::ratio<1>>(duration).count();
        testsuite.append_attribute("time").set_value(RoundTo(doubleTime, precision).c_str());

        totalTests += scenarioTests;
        totalFailures += scenarioFailures;
        totalSkipped += scenarioSkipped;

        testsuite.append_attribute("tests").set_value(scenarioTests);
        testsuite.append_attribute("failures").set_value(scenarioFailures);
        testsuite.append_attribute("skipped").set_value(scenarioSkipped);
    }

    void JunitReport::RuleStart(const engine::RuleInfo& ruleInfo)
    {
        /* do nothing */
    }

    void JunitReport::RuleEnd(engine::Result result, const engine::RuleInfo& ruleInfo, TraceTime::Duration duration)
    {
        /* do nothing */
    }

    void JunitReport::ScenarioStart(const engine::ScenarioInfo& scenarioInfo)
    {
        testcase = testsuite.append_child("testcase");

        testcase.append_attribute("name").set_value(scenarioInfo.Title().c_str());

        ++scenarioTests;
    }

    void JunitReport::ScenarioEnd(engine::Result result, const engine::ScenarioInfo& scenarioInfo, TraceTime::Duration duration)
    {
        const auto doubleTime = std::chrono::duration<double, std::ratio<1>>(duration).count();
        testcase.append_attribute("time").set_value(RoundTo(doubleTime, precision).c_str());

        switch (result)
        {
            case engine::Result::passed:
                break;

            case engine::Result::skipped:
            case engine::Result::pending:
            case engine::Result::ambiguous:
            case engine::Result::undefined:
            {
                ++scenarioSkipped;
                auto skipped = testcase.append_child("skipped");

                if (result == engine::Result::skipped)
                {
                    skipped.append_attribute("message").set_value("Test is skipped due to previous errors.");
                }
                else if (result == engine::Result::undefined)
                {
                    skipped.append_attribute("message").set_value("Test is undefined.");
                }
                else if (result == engine::Result::pending)
                {
                    skipped.append_attribute("message").set_value("Test is pending.");
                }
                else
                {
                    skipped.append_attribute("message").set_value("Test result unkown.");
                }
            }

            break;

            case engine::Result::failed:
                ++scenarioFailures;
                break;
        }

        totalTime += duration;
    }

    void JunitReport::StepSkipped(const engine::StepInfo& stepInfo)
    {
        /* do nothing */
    }

    void JunitReport::StepStart(const engine::StepInfo& stepInfo)
    {
        /* do nothing */
    }

    void JunitReport::StepEnd(engine::Result result, const engine::StepInfo& stepInfo, TraceTime::Duration duration)
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
