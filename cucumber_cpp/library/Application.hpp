#ifndef CUCUMBER_CPP_APPLICATION_HPP
#define CUCUMBER_CPP_APPLICATION_HPP

// IWYU pragma: private, include "cucumber_cpp/CucumberCpp.hpp"
// IWYU pragma: friend cucumber_cpp/.*

#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/Duration.hpp"
#include "cucumber_cpp/library/support/Timestamp.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Validators.hpp>
#include <cstddef>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace cucumber_cpp::library
{
    // struct ReportHandlerValidator : public CLI::Validator
    // {
    //     explicit ReportHandlerValidator(const report::Reporters& reporters);
    // };

    struct Application
    {
        struct Options
        {
            std::set<std::string> paths{};

            bool dryRun{ false };
            bool failFast{ false };

            std::set<std::string> format{};
            std::set<std::string> formatOptions{};

            std::string language{ "en" };

            enum support::RunOptions::Ordering ordering{ support::RunOptions::Ordering::defined };

            std::size_t retry{ 0 };
            std::vector<std::string> retryTagFilter{};

            bool strict{ true };

            std::vector<std::string> tags{};
        };

        explicit Application(std::shared_ptr<ContextStorageFactory> contextStorageFactory = std::make_shared<ContextStorageFactoryImpl>(), bool removeDefaultGoogleTestListener = true);

        int Run(int argc, const char* const* argv);

        CLI::App& CliParser();
        Context& ProgramContext();
        cucumber_expression::ParameterRegistry& ParameterRegistration();

        // void AddReportHandler(const std::string& name, std::unique_ptr<report::ReportHandlerV2>&& reporter);

    private:
        void DryRunFeatures();
        void RunFeatures();
        void PrintStepsNotUsed(const StepRegistry& stepRegistry) const;

        [[nodiscard]] int GetExitCode() const;

        Options options;

        CLI::App cli;
        CLI::App* runCommand;

        std::shared_ptr<ContextStorageFactory> contextStorageFactory;
        std::unique_ptr<Context> programContext{ std::make_unique<Context>(contextStorageFactory) };
        Context& programContextRef{ *programContext };

        util::Broadcaster broadcaster;

        // ReportHandlerValidator reportHandlerValidator;

        cucumber_expression::ParameterRegistry parameterRegistry{};
        bool removeDefaultGoogleTestListener;
        support::StopWatchHighResolutionClock stopwatchHighResolutionClock;
        support::TimestampGeneratorSystemClock timestampGeneratorSystemClock;

        bool runPassed{ false };
    };
}

#endif
