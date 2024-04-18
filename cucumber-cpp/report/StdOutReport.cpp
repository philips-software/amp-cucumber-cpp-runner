#include "cucumber-cpp/report/StdOutReport.hpp"
#include "cucumber-cpp/FeatureRunner.hpp"
#include "cucumber-cpp/ScenarioRunner.hpp"
#include "cucumber-cpp/StepRunner.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <ranges>
#include <ratio>
#include <sstream>

namespace cucumber_cpp::report
{
    namespace
    {
        std::string ConstructLineWithResult(const std::string& text, const std::string& result, std::uint32_t indent, std::uint32_t spacing)
        {
            std::ostringstream out;

            const auto size = text.size() + indent;
            auto dots = std::max(static_cast<int>(spacing - size), 5);

            out << "\n";
            for (; indent != 0; --indent)
                out << ' ';

            out << text;
            for (; dots != 0; --dots)
                out << '.';

            out << result;

            return out.str();
        }

        void PrintLineWithResult(const std::string& text, const std::string& result, std::uint32_t indent, std::uint32_t spacing)
        {
            std::cout << ConstructLineWithResult(text, result, indent, spacing);
        }

        const std::map<report::ReportHandler::Result, std::string> successLut{
            { report::ReportHandler::Result::success, "done" },
            { report::ReportHandler::Result::skipped, "skipped" },
            { report::ReportHandler::Result::failed, "failed" },
            { report::ReportHandler::Result::error, "error" },
            { report::ReportHandler::Result::pending, "pending" },
            { report::ReportHandler::Result::ambiguous, "ambiguous" },
            { report::ReportHandler::Result::undefined, "undefined" },
        };

        std::string ScaledDuration(TraceTime::Duration duration)
        {
            std::ostringstream out;

            if (duration < std::chrono::microseconds{ 1 })
                out << std::chrono::duration<double, std::chrono::nanoseconds::period>(duration);
            else if (duration < std::chrono::milliseconds{ 1 })
                out << std::chrono::duration<double, std::chrono::microseconds::period>(duration);
            else if (duration < std::chrono::seconds{ 1 })
                out << std::chrono::duration<double, std::chrono::milliseconds::period>(duration);
            else if (duration < std::chrono::minutes{ 1 })
                out << std::chrono::duration<double, std::chrono::seconds::period>(duration);
            else if (duration < std::chrono::hours{ 1 })
                out << std::chrono::duration<double, std::chrono::minutes::period>(duration);
            else
                out << std::chrono::duration<double, std::chrono::hours::period>(duration);

            return out.str();
        }
    }

    void StdOutReport::FeatureStart(const FeatureSource& featureSource)
    {
        /* do nothing */
    }

    void StdOutReport::FeatureEnd(const FeatureSource& featureSource, Result result, TraceTime::Duration duration)
    {
        /* do nothing */
    }

    void StdOutReport::ScenarioStart(const ScenarioSource& scenarioSource)
    {
        std::cout << "\n"
                  << scenarioSource.name;
    }

    void StdOutReport::ScenarioEnd(const ScenarioSource& scenarioSource, Result result, TraceTime::Duration duration)
    {
        std::cout << "\n";
    }

    void StdOutReport::StepStart(const StepSource& stepSource)
    {
        std::cout << "\n  " << stepSource.type + " " + stepSource.name;
    }

    void StdOutReport::StepEnd(const StepSource& stepSource, Result result, TraceTime::Duration duration)
    {
        if (result == decltype(result)::success || result == decltype(result)::skipped)
            std::cout << "\n  -> " << successLut.at(result) << " (" << ScaledDuration(duration) << ")";
        else
            std::cout << "\n  -> " << stepSource.scenarioSource.featureSource.path << ":" << stepSource.line << ":" << stepSource.column << ": " << successLut.at(result) << " (" << ScaledDuration(duration) << ")";
    }

    void StdOutReport::Failure(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
        if (path && line && column)
            std::cout
                << "\n"
                << path.value().string() << ":" << line.value() << ":" << column.value() << ": Failure\n"
                << error;
        else
            std::cout
                << "\n"
                << error;
    }

    void StdOutReport::Error(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
        if (path && line && column)
            std::cout
                << "\n"
                << path.value().string() << ":" << line.value() << ":" << column.value() << ": Error\n"
                << error;
        else
            std::cout
                << "\n"
                << error;
    }

    void StdOutReport::Trace(const std::string& trace)
    {
        std::cout << trace;
    }
}
