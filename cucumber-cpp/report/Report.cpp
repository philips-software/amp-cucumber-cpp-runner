#include "cucumber-cpp/report/Report.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "cucumber-cpp/engine/FeatureInfo.hpp"
#include "cucumber-cpp/engine/Result.hpp"
#include "cucumber-cpp/engine/RuleInfo.hpp"
#include "cucumber-cpp/engine/ScenarioInfo.hpp"
#include "cucumber-cpp/engine/StepInfo.hpp"
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::report
{
    namespace
    {
        template<class TFn, class... TArgs>
        void ForwardCall(auto& reporters, TFn fn, TArgs&&... args)
        {
            std::ranges::for_each(
                reporters, [&](const auto& reportHandlerV2)
                {
                    (*reportHandlerV2.*fn)(std::forward<TArgs>(args)...);
                });
        }
    }

    void Reporters::Add(const std::string& name, std::unique_ptr<ReportHandlerV2> reporter)
    {
        availableReporters[name] = std::move(reporter);
    }

    void Reporters::Use(const std::string& name)
    {
        if (availableReporters[name])
            Add(std::move(availableReporters[name]));
    }

    void Reporters::Add(std::unique_ptr<ReportHandlerV2> report)
    {
        reporters.push_back(std::move(report));
    }

    std::vector<std::string> Reporters::AvailableReporters() const
    {
        auto range = std::views::keys(availableReporters);
        return { range.begin(), range.end() };
    }

    std::vector<std::unique_ptr<ReportHandlerV2>>& Reporters::Storage()
    {
        return reporters;
    }

    void ReportForwarder::FeatureStart(const engine::FeatureInfo& featureInfo)
    {
        ForwardCall(Storage(), &ReportHandlerV2::FeatureStart, featureInfo);
    }

    void ReportForwarder::FeatureEnd(engine::Result result, const engine::FeatureInfo& featureInfo, TraceTime::Duration duration)
    {
        ForwardCall(Storage(), &ReportHandlerV2::FeatureEnd, result, featureInfo, duration);
    }

    void ReportForwarder::RuleStart(const engine::RuleInfo& ruleInfo)
    {
        ForwardCall(Storage(), &ReportHandlerV2::RuleStart, ruleInfo);
    }

    void ReportForwarder::RuleEnd(engine::Result result, const engine::RuleInfo& ruleInfo, TraceTime::Duration duration)
    {
        ForwardCall(Storage(), &ReportHandlerV2::RuleEnd, result, ruleInfo, duration);
    }

    void ReportForwarder::ScenarioStart(const engine::ScenarioInfo& scenarioInfo)
    {
        ForwardCall(Storage(), &ReportHandlerV2::ScenarioStart, scenarioInfo);
    }

    void ReportForwarder::ScenarioEnd(engine::Result result, const engine::ScenarioInfo& scenarioInfo, TraceTime::Duration duration)
    {
        ForwardCall(Storage(), &ReportHandlerV2::ScenarioEnd, result, scenarioInfo, duration);
    }

    void ReportForwarder::StepSkipped(const engine::StepInfo& stepInfo)
    {
        ForwardCall(Storage(), &ReportHandlerV2::StepSkipped, stepInfo);
    }

    void ReportForwarder::StepStart(const engine::StepInfo& stepInfo)
    {
        ForwardCall(Storage(), &ReportHandlerV2::StepStart, stepInfo);
    }

    void ReportForwarder::StepEnd(engine::Result result, const engine::StepInfo& stepInfo, TraceTime::Duration duration)
    {
        ForwardCall(Storage(), &ReportHandlerV2::StepEnd, result, stepInfo, duration);
    }

    void ReportForwarder::Failure(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
        ForwardCall(Storage(), &ReportHandlerV2::Failure, error, path, line, column);
    }

    void ReportForwarder::Error(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
        ForwardCall(Storage(), &ReportHandlerV2::Error, error, path, line, column);
    }

    void ReportForwarder::Trace(const std::string& trace)
    {
        ForwardCall(Storage(), &ReportHandlerV2::Trace, trace);
    }
}
