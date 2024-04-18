#ifndef CUCUMBER_CPP_APPLICATION_HPP
#define CUCUMBER_CPP_APPLICATION_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/CucumberRunner.hpp"
#include "cucumber-cpp/FeatureRunner.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include "cucumber/gherkin/app.hpp"
#include <CLI/CLI.hpp>
#include <filesystem>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp
{
    struct ReportHandlerValidator : public CLI::Validator
    {
        explicit ReportHandlerValidator(const report::Reporters& reporters);
    };

    struct ResultStatus
    {
        using Result = cucumber_cpp::report::ReportHandler::Result;

        ResultStatus& operator=(Result result);
        explicit operator Result() const;

        [[nodiscard]] bool IsSuccess() const;

    private:
        Result resultStatus{ Result::undefined };
    };

    struct Application
    {
        struct Options
        {
            std::vector<std::string> tags{};
            std::vector<std::string> features{};
            std::vector<std::string> reporters{};

            std::string outputfolder{ "./out" };
            std::string reportfile{ "TestReport" };
        };

        explicit Application(std::shared_ptr<ContextStorageFactory> contextStorageFactory = std::make_shared<ContextStorageFactoryImpl>());

        int Run(int argc, const char* const* argv);

        CLI::App& CliParser();
        Context& ProgramContext();
        const Options& CliOptions() const;

        void AddReportHandler(const std::string& name, std::unique_ptr<report::ReportHandler>&& reporter);

    private:
        [[nodiscard]] int GetExitCode() const;
        [[nodiscard]] std::vector<std::filesystem::path> GetFeatureFiles() const;
        void RunFeatures();
        [[nodiscard]] report::ReportHandler::Result RunFeature(CucumberRunner& cucumberRunner, const std::filesystem::path& path);

        Options options;
        CLI::App cli;
        CLI::App* runCommand;

        Context programContext;

        report::ReportForwarder reporters;
        ReportHandlerValidator reportHandlerValidator;

        cucumber::gherkin::app gherkin;
        ResultStatus resultStatus;
    };
}

#endif
