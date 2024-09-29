#ifndef REPORT_REPORT_HPP
#define REPORT_REPORT_HPP

#include "cucumber_cpp/TraceTime.hpp"
#include "cucumber_cpp/engine/FeatureInfo.hpp"
#include "cucumber_cpp/engine/Result.hpp"
#include "cucumber_cpp/engine/RuleInfo.hpp"
#include "cucumber_cpp/engine/StepInfo.hpp"
#include "cucumber_cpp/engine/TestRunner.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace cucumber_cpp
{
    struct FeatureSource;
    struct ScenarioSource;
    struct StepSource;
}

namespace cucumber_cpp::report
{
    struct ReportHandlerV2
    {
        virtual ~ReportHandlerV2() = default;

        virtual void FeatureStart(const engine::FeatureInfo& featureInfo) = 0;
        virtual void FeatureEnd(engine::Result result, const engine::FeatureInfo& featureInfo, TraceTime::Duration duration) = 0;

        virtual void RuleStart(const engine::RuleInfo& ruleInfo) = 0;
        virtual void RuleEnd(engine::Result result, const engine::RuleInfo& ruleInfo, TraceTime::Duration duration) = 0;

        virtual void ScenarioStart(const engine::ScenarioInfo& scenarioInfo) = 0;
        virtual void ScenarioEnd(engine::Result result, const engine::ScenarioInfo& scenarioInfo, TraceTime::Duration duration) = 0;

        virtual void StepSkipped(const engine::StepInfo& stepInfo) = 0;
        virtual void StepStart(const engine::StepInfo& stepInfo) = 0;
        virtual void StepEnd(engine::Result result, const engine::StepInfo& stepInfo, TraceTime::Duration duration) = 0;

        virtual void Failure(const std::string& error, std::optional<std::filesystem::path> path = {}, std::optional<std::size_t> line = {}, std::optional<std::size_t> column = {}) = 0;
        virtual void Error(const std::string& error, std::optional<std::filesystem::path> path = {}, std::optional<std::size_t> line = {}, std::optional<std::size_t> column = {}) = 0;

        virtual void Trace(const std::string& trace) = 0;

        virtual void Summary(TraceTime::Duration duration) = 0;
    };

    struct ReportHandler
    {
        enum struct Result
        {
            success,
            skipped,
            failed,
            error,
            pending,
            ambiguous,
            undefined,
        };

        virtual ~ReportHandler() = default;

        virtual void FeatureStart(const FeatureSource& featureSource) = 0;
        virtual void FeatureEnd(const FeatureSource& featureSource, Result result, TraceTime::Duration duration) = 0;

        virtual void ScenarioStart(const ScenarioSource& scenarioSource) = 0;
        virtual void ScenarioEnd(const ScenarioSource& scenarioSource, Result result, TraceTime::Duration duration) = 0;

        virtual void StepStart(const StepSource& stepSource) = 0;
        virtual void StepEnd(const StepSource& stepSource, Result result, TraceTime::Duration duration) = 0;

        virtual void Failure(const std::string& error, std::optional<std::filesystem::path> path = {}, std::optional<std::size_t> line = {}, std::optional<std::size_t> column = {}) = 0;
        virtual void Error(const std::string& error, std::optional<std::filesystem::path> path = {}, std::optional<std::size_t> line = {}, std::optional<std::size_t> column = {}) = 0;

        virtual void Trace(const std::string& trace) = 0;
    };

    struct Reporters
    {
        void Add(const std::string& name, std::unique_ptr<ReportHandlerV2> reporter);
        void Use(const std::string& name);

        [[nodiscard]] std::vector<std::string> AvailableReporters() const;

    protected:
        void Add(std::unique_ptr<ReportHandlerV2> report);
        std::vector<std::unique_ptr<ReportHandlerV2>>& Storage();

    private:
        std::map<std::string, std::unique_ptr<ReportHandlerV2>, std::less<>> availableReporters;
        std::vector<std::unique_ptr<ReportHandlerV2>> reporters;
    };

    struct ReportForwarder
        : Reporters
        , ReportHandlerV2
    {
        void FeatureStart(const engine::FeatureInfo& featureInfo) override;
        void FeatureEnd(engine::Result result, const engine::FeatureInfo& featureInfo, TraceTime::Duration duration) override;

        void RuleStart(const engine::RuleInfo& ruleInfo) override;
        void RuleEnd(engine::Result result, const engine::RuleInfo& ruleInfo, TraceTime::Duration duration) override;

        void ScenarioStart(const engine::ScenarioInfo& scenarioInfo) override;
        void ScenarioEnd(engine::Result result, const engine::ScenarioInfo& scenarioInfo, TraceTime::Duration duration) override;

        void StepSkipped(const engine::StepInfo& stepInfo) override;
        void StepStart(const engine::StepInfo& stepInfo) override;
        void StepEnd(engine::Result result, const engine::StepInfo& stepInfo, TraceTime::Duration duration) override;

        void Failure(const std::string& error, std::optional<std::filesystem::path> path = {}, std::optional<std::size_t> line = {}, std::optional<std::size_t> column = {}) override;
        void Error(const std::string& error, std::optional<std::filesystem::path> path = {}, std::optional<std::size_t> line = {}, std::optional<std::size_t> column = {}) override;

        void Trace(const std::string& trace) override;

        void Summary(TraceTime::Duration duration) override;
    };
}

#endif
