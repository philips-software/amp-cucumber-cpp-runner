#ifndef REPORT_STDOUTREPORT_HPP
#define REPORT_STDOUTREPORT_HPP

#include "cucumber-cpp/TraceTime.hpp"
#include "cucumber-cpp/engine/FeatureInfo.hpp"
#include "cucumber-cpp/engine/Result.hpp"
#include "cucumber-cpp/engine/RuleInfo.hpp"
#include "cucumber-cpp/engine/ScenarioInfo.hpp"
#include "cucumber-cpp/engine/StepInfo.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace cucumber_cpp::report
{
    struct StdOutReport : ReportHandlerV2
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

    private:
        std::uint32_t nrOfScenarios{ 0 };
        std::vector<const engine::ScenarioInfo*> failedScenarios;
    };
}

#endif
