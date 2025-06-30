#ifndef CUCUMBER_CPP_APPLICATION_HPP
#define CUCUMBER_CPP_APPLICATION_HPP

#include "cucumber/gherkin/app.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FeatureFactory.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Validators.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library
{
    struct ReportHandlerValidator : public CLI::Validator
    {
        explicit ReportHandlerValidator(const report::Reporters& reporters);
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
        cucumber_expression::ParameterRegistration& ParameterRegistration();

        void AddReportHandler(const std::string& name, std::unique_ptr<report::ReportHandlerV2>&& reporter);

    private:
        [[nodiscard]] int GetExitCode() const;
        [[nodiscard]] int GetExitCode(engine::Result result) const;
        void DryRunFeatures();
        void RunFeatures();
        [[nodiscard]] std::vector<std::unique_ptr<engine::FeatureInfo>> GetFeatureTree(const engine::FeatureTreeFactory& featureTreeFactory, std::string_view tagExpression);
        [[nodiscard]] engine::Result RunFeature(const std::filesystem::path& path, std::string_view tagExpression, report::ReportHandlerV2& reportHandler);

        Options options;
        CLI::App cli;
        CLI::App* runCommand;

        engine::ContextManager contextManager;

        report::ReportForwarderImpl reporters;
        ReportHandlerValidator reportHandlerValidator;

        cucumber::gherkin::app gherkin;

        cucumber_expression::ParameterRegistry parameterRegistry;
    };
}

#endif
