#include "NdjsonReport.hpp"

namespace cucumber_cpp::library::report
{
    namespace
    {

    }

    NdjsonReport::NdjsonReport(const std::string& outputfolder, const std::string& reportfile)
    {

    }

    NdjsonReport::~NdjsonReport()
    = default;

    void NdjsonReport::FeatureStart(const engine::FeatureInfo& featureInfo)
    {

    }

    void NdjsonReport::FeatureEnd(engine::Result result, const engine::FeatureInfo& featureInfo, TraceTime::Duration duration)
    {
    }

    void NdjsonReport::RuleStart(const engine::RuleInfo& ruleInfo)
    {
    }

    void NdjsonReport::RuleEnd(engine::Result result, const engine::RuleInfo& ruleInfo, TraceTime::Duration duration)
    {
    }

    void NdjsonReport::ScenarioStart(const engine::ScenarioInfo& scenarioInfo)
    {
    }

    void NdjsonReport::ScenarioEnd(engine::Result result, const engine::ScenarioInfo& scenarioInfo, TraceTime::Duration duration)
    {
    }

    void NdjsonReport::StepSkipped(const engine::StepInfo& stepInfo)
    {
    }

    void NdjsonReport::StepStart(const engine::StepInfo& stepInfo)
    {
    }

    void NdjsonReport::StepEnd(engine::Result result, const engine::StepInfo& stepInfo, TraceTime::Duration duration)
    {
    }

    void NdjsonReport::Failure(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
    }

    void NdjsonReport::Error(const std::string& error, std::optional<std::filesystem::path> path, std::optional<std::size_t> line, std::optional<std::size_t> column)
    {
    }

    void NdjsonReport::Trace(const std::string& trace)
    {
    }

    void NdjsonReport::Summary(TraceTime::Duration duration)
    {
    }

}
