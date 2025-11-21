#include "cucumber_cpp/library/report/Report.hpp"
#include "cucumber_cpp/library/TraceTime.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::report
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

    void Reporters::Add(const std::string& name, std::function<std::unique_ptr<report::ReportHandlerV2>()> reporterFactory)
    {
        availableReporters[name] = std::move(reporterFactory);
    }

    void Reporters::Use(const std::string& name)
    {
        if (availableReporters[name])
            Add(std::move(availableReporters[name]()));
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

    ReportForwarder::ProgramScope::ProgramScope(cucumber_cpp::library::engine::ProgramContext& programContext, std::vector<std::unique_ptr<ReportHandlerV2>>& reporters)
        : programContext{ programContext }
        , reporters{ reporters }
    {}

    ReportForwarder::ProgramScope::~ProgramScope()
    {
        ForwardCall(reporters, &ReportHandlerV2::Summary, programContext.Duration());
    }

    ReportForwarder::FeatureScope::FeatureScope(cucumber_cpp::library::engine::FeatureContext& featureContext, std::vector<std::unique_ptr<ReportHandlerV2>>& reporters)
        : featureContext{ featureContext }
        , reporters{ reporters }
    {
        ForwardCall(reporters, &ReportHandlerV2::FeatureStart, featureContext.info);
    }

    ReportForwarder::FeatureScope::~FeatureScope()
    {
        ForwardCall(reporters, &ReportHandlerV2::FeatureEnd, featureContext.ExecutionStatus(), featureContext.info, featureContext.Duration());
    }

    ReportForwarder::RuleScope::RuleScope(cucumber_cpp::library::engine::RuleContext& ruleContext, std::vector<std::unique_ptr<ReportHandlerV2>>& reporters)
        : ruleContext{ ruleContext }
        , reporters{ reporters }
    {
        ForwardCall(reporters, &ReportHandlerV2::RuleStart, ruleContext.info);
    }

    ReportForwarder::RuleScope::~RuleScope()
    {
        ForwardCall(reporters, &ReportHandlerV2::RuleEnd, ruleContext.ExecutionStatus(), ruleContext.info, ruleContext.Duration());
    }

    ReportForwarder::ScenarioScope::ScenarioScope(cucumber_cpp::library::engine::ScenarioContext& scenarioContext, std::vector<std::unique_ptr<ReportHandlerV2>>& reporters)
        : scenarioContext{ scenarioContext }
        , reporters{ reporters }
    {
        ForwardCall(reporters, &ReportHandlerV2::ScenarioStart, scenarioContext.info);
    }

    ReportForwarder::ScenarioScope::~ScenarioScope()
    {
        ForwardCall(reporters, &ReportHandlerV2::ScenarioEnd, scenarioContext.ExecutionStatus(), scenarioContext.info, scenarioContext.Duration());
    }

    ReportForwarder::StepScope::StepScope(cucumber_cpp::library::engine::StepContext& stepContext, std::vector<std::unique_ptr<ReportHandlerV2>>& reporters)
        : stepContext{ stepContext }
        , reporters{ reporters }
    {
        ForwardCall(reporters, &ReportHandlerV2::StepStart, stepContext.info);
    }

    ReportForwarder::StepScope::~StepScope()
    {
        ForwardCall(reporters, &ReportHandlerV2::StepEnd, stepContext.ExecutionStatus(), stepContext.info, stepContext.Duration());
    }

    ReportForwarderImpl::ReportForwarderImpl(cucumber_cpp::library::engine::ContextManager& contextManager)
        : contextManager{ contextManager }
    {}

    [[nodiscard]] ReportForwarder::ProgramScope ReportForwarderImpl::ProgramStart()
    {
        return ProgramScope{ contextManager.ProgramContext(), Storage() };
    }

    ReportForwarder::FeatureScope ReportForwarderImpl::FeatureStart()
    {
        return FeatureScope{ contextManager.FeatureContext(), Storage() };
    }

    ReportForwarder::RuleScope ReportForwarderImpl::RuleStart()
    {
        return RuleScope{ contextManager.RuleContext(), Storage() };
    }

    ReportForwarder::ScenarioScope ReportForwarderImpl::ScenarioStart()
    {
        return ScenarioScope{ contextManager.ScenarioContext(), Storage() };
    }

    ReportForwarder::StepScope ReportForwarderImpl::StepStart()
    {
        return StepScope{ contextManager.StepContext(), Storage() };
    }

    void ReportForwarderImpl::StepSkipped()
    {
        ForwardCall(Storage(), &ReportHandlerV2::StepSkipped, contextManager.StepContext().info);
    }

    void ReportForwarderImpl::Failure(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
        ForwardCall(Storage(), &ReportHandlerV2::Failure, error, path, line, column);
    }

    void ReportForwarderImpl::Error(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
        ForwardCall(Storage(), &ReportHandlerV2::Error, error, path, line, column);
    }

    void ReportForwarderImpl::Trace(const std::string& trace)
    {
        ForwardCall(Storage(), &ReportHandlerV2::Trace, trace);
    }

    void ReportForwarderImpl::Summary(TraceTime::Duration duration)
    {
        ForwardCall(Storage(), &ReportHandlerV2::Summary, duration);
    }
}
