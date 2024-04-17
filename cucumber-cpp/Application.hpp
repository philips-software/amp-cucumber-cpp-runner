#ifndef CUCUMBER_CPP_APPLICATION_HPP
#define CUCUMBER_CPP_APPLICATION_HPP

#include "CLI/CLI.hpp"
#include "args.hxx"
#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/CucumberRunner.hpp"
#include "cucumber-cpp/FeatureRunner.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include "cucumber/gherkin/app.hpp"
#include <CLI/App.hpp>
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
    struct GherkinParser
    {
        explicit GherkinParser(CucumberRunnerV2& cucumberRunner);

        [[nodiscard]] report::ReportHandler::Result RunFeatureFile(const std::filesystem::path& path);

    private:
        CucumberRunnerV2& cucumberRunner;
        std::unique_ptr<FeatureRunnerV2> featureRunner;

        cucumber::gherkin::app gherkin;
        cucumber::gherkin::app::callbacks callbacks;
    };

    struct Application
    {
        Application();

        int Run(int argc, const char* const* argv);
        int Run(int argc, const char* const* argv, std::shared_ptr<ContextStorageFactory> contextStorageFactory);

        CLI::App& CliParser();

        void AddReportHandler(const std::string& name, std::unique_ptr<report::ReportHandler>&& reporter);

    private:
        [[nodiscard]] int GetExitCode() const;
        [[nodiscard]] std::vector<std::filesystem::path> GetFeatureFiles() const;
        void RunFeatures(std::shared_ptr<ContextStorageFactory> contextStorageFactory = std::make_shared<ContextStorageFactoryImpl>());

        struct Options
        {
            std::vector<std::string_view> tags{};
            std::vector<std::string_view> features{};
            std::vector<std::string_view> reporters{};
        };

        Options options{};
        CLI::App cli{ "name" };

        report::ReportForwarder reporters;
        report::ReportHandler::Result result{ report::ReportHandler::Result::undefined };
    };
}

#endif
