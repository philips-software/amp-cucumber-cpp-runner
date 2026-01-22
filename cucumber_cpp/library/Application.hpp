#ifndef CUCUMBER_CPP_APPLICATION_HPP
#define CUCUMBER_CPP_APPLICATION_HPP

// IWYU pragma: private, include "cucumber_cpp/CucumberCpp.hpp"
// IWYU pragma: friend cucumber_cpp/.*

#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/api/Formatters.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Validators.hpp>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace cucumber_cpp::library
{
    struct Application
    {
        struct Options
        {
            bool dumpConfig{ false };

            std::set<std::string, std::less<>> paths{ { (std::filesystem::path(".") / "features").string() } };

            bool dryRun{ false };
            bool failFast{ false };

            std::set<std::string, std::less<>> format{ "summary" };
            std::string formatOptions{ R"({})" };

            std::string language{ "en" };

            enum support::RunOptions::Ordering ordering{ support::RunOptions::Ordering::defined };

            std::size_t retry{ 0 };
            std::vector<std::string> retryTagFilter{};

            bool strict{ true };

            bool recursive{ true };

            std::vector<std::string> tags{};
        };

        Application(std::shared_ptr<ContextStorageFactory> contextStorageFactory = std::make_shared<ContextStorageFactoryImpl>(), bool removeDefaultGoogleTestListener = true);

        [[nodiscard]] int Run(int argc, const char* const* argv);

        CLI::App& CliParser();
        Context& ProgramContext();
        cucumber_expression::ParameterRegistry& ParameterRegistration();
        api::Formatters& Formatters();

    private:
        void DryRunFeatures();
        [[nodiscard]] int RunFeatures();

        Options options;

        CLI::App cli;

        std::shared_ptr<ContextStorageFactory> contextStorageFactory;
        std::unique_ptr<Context> programContext{ std::make_unique<Context>(contextStorageFactory) };
        Context& programContextRef{ *programContext };

        api::Formatters formatters;

        util::Broadcaster broadcaster;

        cucumber_expression::ParameterRegistry parameterRegistry{ cucumber_cpp::library::support::DefinitionRegistration::Instance().GetRegisteredParameters() };
        bool removeDefaultGoogleTestListener;
        util::StopWatchHighResolutionClock stopwatchHighResolutionClock;
        util::TimestampGeneratorSystemClock timestampGeneratorSystemClock;

        bool runPassed{ false };
    };
}

#endif
