#ifndef REPORT_REPORT_HPP
#define REPORT_REPORT_HPP

#include "cucumber-cpp/TraceTime.hpp"
#include <cstddef>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace cucumber_cpp
{
    struct FeatureSource;
    struct ScenarioSource;
    struct StepSource;
}

namespace cucumber_cpp::report
{
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
        void Add(const std::string& name, std::unique_ptr<ReportHandler>&& reporter);
        void Use(const std::string& name);
        void Add(std::unique_ptr<ReportHandler>&& report);

        std::vector<std::string> AvailableReporters() const;

    protected:
        std::vector<std::unique_ptr<ReportHandler>>& Storage();

    private:
        std::map<std::string, std::unique_ptr<ReportHandler>, std::less<>> availableReporters;
        std::vector<std::unique_ptr<ReportHandler>> reporters;
    };

    struct ReportForwarder
        : Reporters
        , ReportHandler
    {
        void FeatureStart(const FeatureSource& featureSource) override;
        void FeatureEnd(const FeatureSource& featureSource, Result result, TraceTime::Duration duration) override;

        void ScenarioStart(const ScenarioSource& scenarioSource) override;
        void ScenarioEnd(const ScenarioSource& scenarioSource, Result result, TraceTime::Duration duration) override;

        void StepStart(const StepSource& stepSource) override;
        void StepEnd(const StepSource& stepSource, Result result, TraceTime::Duration duration) override;

        void Failure(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column) override;
        void Error(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column) override;

        void Trace(const std::string& trace) override;
    };
}

#endif
