#ifndef CUCUMBER_CPP_APPLICATION_HPP
#define CUCUMBER_CPP_APPLICATION_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/CucumberRunner.hpp"
#include "cucumber-cpp/FeatureRunner.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include "cucumber/gherkin/app.hpp"
#include <filesystem>
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

        cucumber::gherkin::app app;
        cucumber::gherkin::app::callbacks cbs;
    };

    struct Application
    {
        Application(std::span<const char*> args);

        [[nodiscard]] const std::vector<std::string_view>& GetForwardArgs() const;

        void RunFeatures(std::shared_ptr<ContextStorageFactory> contextStorageFactory = std::make_shared<ContextStorageFactoryImpl>());

        [[nodiscard]] int GetExitCode() const;

        [[nodiscard]] report::Reporters& Reporters();

    private:
        [[nodiscard]] std::vector<std::filesystem::path> GetFeatureFiles() const;

        struct Options
        {
            explicit Options(std::span<const char*> args);

            std::vector<std::string_view> tags{};
            std::vector<std::string_view> features{};
            std::vector<std::string_view> reports{};
            std::vector<std::string_view> forwardArgs{};
        };

        Options options;

        report::ReportForwarder reporters;
        report::ReportHandler::Result result{ report::ReportHandler::Result::undefined };
    };
}

#endif
