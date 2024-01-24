#include "cucumber-cpp/report/Report.hpp"
#include "cucumber-cpp/FeatureRunner.hpp"
#include "cucumber-cpp/ScenarioRunner.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
#include <source_location>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace cucumber_cpp::report
{
    namespace
    {
        template<class TFn, class... TArgs>
        void ForwardCall(auto& reporters, TFn fn, TArgs&&... args)
        {
            std::ranges::for_each(
                reporters, [&](const auto& reportHandler)
                {
                    (*reportHandler.*fn)(std::forward<TArgs>(args)...);
                });
        }
    }

    void Reporters::Add(std::unique_ptr<ReportHandler>&& report)
    {
        reporters.push_back(std::move(report));
    }

    std::vector<std::unique_ptr<ReportHandler>>& Reporters::Storage()
    {
        return reporters;
    }

    void ReportForwarder::FeatureStart(const FeatureSource& featureSource)
    {
        ForwardCall(Storage(), &ReportHandler::FeatureStart, featureSource);
    }

    void ReportForwarder::FeatureEnd(const FeatureSource& featureSource, Result result, TraceTime::Duration duration)
    {
        ForwardCall(Storage(), &ReportHandler::FeatureEnd, featureSource, result, duration);
    }

    void ReportForwarder::ScenarioStart(const ScenarioSource& scenarioSource)
    {
        ForwardCall(Storage(), &ReportHandler::ScenarioStart, scenarioSource);
    }

    void ReportForwarder::ScenarioEnd(const ScenarioSource& scenarioSource, Result result, TraceTime::Duration duration)
    {
        ForwardCall(Storage(), &ReportHandler::ScenarioEnd, scenarioSource, result, duration);
    }

    void ReportForwarder::StepStart(const StepSource& stepSource)
    {
        ForwardCall(Storage(), &ReportHandler::StepStart, stepSource);
    }

    void ReportForwarder::StepEnd(const StepSource& stepSource, Result result, TraceTime::Duration duration)
    {
        ForwardCall(Storage(), &ReportHandler::StepEnd, stepSource, result, duration);
    }

    void ReportForwarder::Failure(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
        ForwardCall(Storage(), &ReportHandler::Failure, error, path, line, column);
    }

    void ReportForwarder::Error(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
        ForwardCall(Storage(), &ReportHandler::Error, error, path, line, column);
    }

    void ReportForwarder::Trace(const std::string& trace)
    {
        ForwardCall(Storage(), &ReportHandler::Trace, trace);
    }
}
