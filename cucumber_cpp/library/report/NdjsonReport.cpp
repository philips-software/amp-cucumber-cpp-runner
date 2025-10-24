#include "NdjsonReport.hpp"
#include <fstream>

namespace cucumber_cpp::library::report
{
    namespace
    {

    }

    NdjsonReport::NdjsonReport(const std::string& outputFolder, const std::string& reportFile)
        : outputFolder(outputFolder)
        , reportFile(reportFile)
    {
    }

    NdjsonReport::~NdjsonReport()
    {
        try
        {
            if (!std::filesystem::exists(outputFolder))
            {
                std::filesystem::create_directories(outputFolder);

                const std::filesystem::path outputFile = std::filesystem::path{ outputFolder }.append(reportFile + ".ndjson");
                std::ofstream out(outputFile);
                for (nlohmann::json& jsonEvent : docs)
                {
                    out << jsonEvent;
                }

                out.close();
            }
        }
        catch (const std::filesystem::filesystem_error& ex)
        {
            std::cout << "\nwhat():  " << ex.what() << '\n'
                      << "path1(): " << ex.path1() << '\n'
                      << "path2(): " << ex.path2() << '\n'
                      << "code().value():    " << ex.code().value() << '\n'
                      << "code().message():  " << ex.code().message() << '\n'
                      << "code().category(): " << ex.code().category().name() << '\n';
        }
    }

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
