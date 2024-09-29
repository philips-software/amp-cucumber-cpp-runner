#ifndef CUCUMBER_CPP_APPLICATION_HPP
#define CUCUMBER_CPP_APPLICATION_HPP

#include "cucumber/gherkin/app.hpp"
#include "cucumber_cpp/Context.hpp"
#include "cucumber_cpp/engine/ContextManager.hpp"
#include "cucumber_cpp/engine/FeatureFactory.hpp"
#include "cucumber_cpp/engine/FeatureInfo.hpp"
#include "cucumber_cpp/report/Report.hpp"
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Validators.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
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

            bool dryrun{ false };
        };

        explicit Application(std::shared_ptr<ContextStorageFactory> contextStorageFactory = std::make_shared<ContextStorageFactoryImpl>());

        int Run(int argc, const char* const* argv);

        CLI::App& CliParser();
        Context& ProgramContext();
        const Options& CliOptions() const;

        void AddReportHandler(const std::string& name, std::unique_ptr<report::ReportHandlerV2>&& reporter);

    private:
        [[nodiscard]] int GetExitCode() const;
        [[nodiscard]] std::vector<std::filesystem::path> GetFeatureFiles() const;
        void DryRunFeatures();
        void RunFeatures();
        [[nodiscard]] std::vector<std::unique_ptr<engine::FeatureInfo>> GetFeatureTree(std::string_view tagExpression);
        [[nodiscard]] report::ReportHandler::Result RunFeature(const std::filesystem::path& path, std::string_view tagExpression, report::ReportHandlerV2& reportHandler);

        Options options;
        CLI::App cli;
        CLI::App* runCommand;

        report::ReportForwarder reporters;
        ReportHandlerValidator reportHandlerValidator;

        cucumber::gherkin::app gherkin;

        engine::FeatureTreeFactory featureTreeFactory{};
        engine::ContextManager contextManager;
    };
}

#endif
