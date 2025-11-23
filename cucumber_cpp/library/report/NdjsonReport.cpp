#include "NdjsonReport.hpp"
#include "Environment.h"
#include <cucumber/messages/meta.hpp>
#include <cucumber_cpp/library/engine/SourceInfo.hpp>
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
        outStream.close();
    }


    /*
     * Unlike XML reporter, where a single document is built up over time and then has to be flushed out in the end all at once,
     * NDJSON is a series of individual json objects appended one after another as the test run progresses.
     * Therefore, need to prepare a stream on first call to the reporter
     */
    void NdjsonReport::InitReportDirectory()
    {
        try
        {
            if (!std::filesystem::exists(outputFolder))
            {
                std::filesystem::create_directories(outputFolder);
            }

            const std::filesystem::path outputFile = std::filesystem::path{ outputFolder }.append(reportFile + ".ndjson");
            outStream.open(outputFile, std::ios::trunc);
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

    void NdjsonReport::CreateMeta()
    {
        // Makeup own metadata since https://github.com/cucumber/ci-environment does not have c++ version (yet)
        cucumber::messages::meta meta (MESSAGES_PROTOCOL_VERSION);
    }


    void NdjsonReport::FeatureStart(const engine::FeatureInfo& featureInfo)
    {
        InitReportDirectory();
        CreateMeta();

        outStream << "{\"source\":" << featureInfo.SourceInfo()->ToJson() << "}\n";
        outStream << "{\"gherkinDocument\":" << featureInfo.ToJson() << "}\n";
        // TODO testRunStarted
    }

    void NdjsonReport::FeatureEnd(engine::Result result, const engine::FeatureInfo& featureInfo, TraceTime::Duration duration)
    {
        // TODO testRunFinished
    }

    void NdjsonReport::RuleStart(const engine::RuleInfo& ruleInfo)
    {
    }

    void NdjsonReport::RuleEnd(engine::Result result, const engine::RuleInfo& ruleInfo, TraceTime::Duration duration)
    {
    }

    void NdjsonReport::ScenarioStart(const engine::ScenarioInfo& scenarioInfo)
    {
        // each scenario is a pickle
        outStream << "{\"pickle\":" << scenarioInfo.ToJson() << "}\n";
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
