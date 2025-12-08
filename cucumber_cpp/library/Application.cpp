#include "cucumber_cpp/library/Application.hpp"
#include "cucumber/messages/envelope.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/Errors.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/Query.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/api/RunCucumber.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/engine/NewRuntime.hpp"
#include "cucumber_cpp/library/formatter/SummaryFormatter.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <CLI/Error.hpp>
#include <CLI/Option.hpp>
#include <CLI/Validators.hpp>
#include <CLI/impl/App_inl.hpp>
#include <CLI/impl/Option_inl.hpp>
#include <exception>
#include <filesystem>
#include <format>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace cucumber_cpp::library
{
    namespace
    {
        template<class Range>
        std::string Join(const Range& range, std::string_view delim)
        {
            if (range.empty())
                return "";

            return std::accumulate(std::next(range.begin()), range.end(), range.front(), [&delim](const auto& lhs, const auto& rhs)
                {
                    return std::format("{}{}{}", lhs, delim, rhs);
                });
        }

        std::filesystem::path ToFileSystemPath(const std::string_view& sv)
        {
            return { sv };
        }

        bool IsFeatureFile(const std::filesystem::directory_entry& entry)
        {
            return std::filesystem::is_regular_file(entry) && entry.path().has_extension() && entry.path().extension() == ".feature";
        }

        std::vector<std::filesystem::path> GetFeatureFiles(Application::Options& options)
        {
            std::vector<std::filesystem::path> files;

            for (const auto feature : options.features | std::views::transform(ToFileSystemPath))
                if (std::filesystem::is_directory(feature))
                    for (const auto& entry : std::filesystem::directory_iterator{ feature } | std::views::filter(IsFeatureFile))
                        files.emplace_back(entry.path());
                else
                    files.emplace_back(feature);

            return files;
        }
    }

    // ReportHandlerValidator::ReportHandlerValidator(const report::Reporters& reporters)
    //     : CLI::Validator("ReportHandler", [&reporters, cachedAvailableReporters = std::optional<std::vector<std::string>>{}](const std::string& str) mutable
    //           {
    //               if (!cachedAvailableReporters)
    //                   cachedAvailableReporters = reporters.AvailableReporters();

    //               if (std::ranges::find(*cachedAvailableReporters, str) == cachedAvailableReporters->end())
    //                   return std::string{ "'" + str + "' is not a reporter" };
    //               else
    //                   return std::string{};
    //           })
    // {}

    Application::Application(std::shared_ptr<ContextStorageFactory> contextStorageFactory, bool removeDefaultGoogleTestListener)
        : contextStorageFactory{ contextStorageFactory }
        , removeDefaultGoogleTestListener{ removeDefaultGoogleTestListener }
    {
        cli.require_subcommand(1);

        runCommand = cli.add_subcommand("run")->parse_complete_callback([this]
            {
                RunFeatures();
            });

        runCommand->add_option("-t,--tag", options.tags, "Cucumber tag expression");
        runCommand->add_option("-f,--feature", options.features, "Feature file or folder with feature files")->required()->check(CLI::ExistingPath);

        runCommand->add_option("-r,--report", options.reporters, "Name of the report generator: ")->required()->group("report generation"); //->check(reportHandlerValidator);
        runCommand->add_option("--outputfolder", options.outputfolder, "Specifies the output folder for generated report files")->group("report generation");
        runCommand->add_option("--reportfile", options.reportfile, "Specifies the output name for generated report files")->group("report generation");
        runCommand->add_flag("--dry", options.dryrun, "Generate report without running tests");
        runCommand->add_flag("--unused", options.printStepsNotUsed, "Show step definitions that were not used");

        // reporters.Add("console", std::make_unique<report::StdOutReport>());
        // reporters.Add("junit-xml", std::make_unique<report::JunitReport>(options.outputfolder, options.reportfile));

        ProgramContext().InsertRef(options);
    }

    int Application::Run(int argc, const char* const* argv)
    {
        try
        {
            const auto reportDescription = runCommand->get_option("--report")->get_description();
            const auto joinedReporters = reportDescription; // + Join(reporters.AvailableReporters(), ", ");

            runCommand->get_option("--report")->description(joinedReporters);
            cli.parse(argc, argv);
        }
        catch (const CLI::ParseError& e)
        {
            return cli.exit(e);
        }
        catch (const InternalError& error)
        {
            std::cout << std::format("InternalError error:\n{}\n", error.what());
            return 1;
        }
        catch (const cucumber_expression::Error& error)
        {
            std::cout << std::format("Cucumber Expression error:\n{}\n", error.what());
            return 1;
        }
        catch (const std::exception& error)
        {
            std::cout << std::format("Generic error:\n{}\n", error.what());
            return 1;
        }
        catch (...)
        {
            std::cout << "Unknown error";
            return 1;
        }

        return GetExitCode();
    }

    CLI::App& Application::CliParser()
    {
        return *runCommand;
    }

    Context& Application::ProgramContext()
    {
        return programContextRef;
    }

    cucumber_expression::ParameterRegistration& Application::ParameterRegistration()
    {
        return parameterRegistry;
    }

    // void Application::AddReportHandler(const std::string& name, std::unique_ptr<report::ReportHandlerV2>&& reporter)
    // {
    //     reporters.Add(name, std::move(reporter));
    // }

    void Application::RunFeatures()
    {
        struct BroadcastListener
        {
            explicit BroadcastListener(util::Broadcaster& broadcaster)
                : listener(broadcaster, [this](const cucumber::messages::envelope& envelope)
                      {
                          OnEvent(envelope);
                      })
            {}

            void OnEvent(const cucumber::messages::envelope& envelope)
            {
                std::cout << envelope.to_json() << "\n";
            }

        private:
            util::Listener listener;
        };

        // BroadcastListener broadcastListener{ broadcaster };

        // for (const auto& selectedReporter : options.reporters)
        //     reporters.Use(selectedReporter);

        const auto tagExpression = Join(options.tags, " ");
        const auto featureFiles = GetFeatureFiles(options);

        support::RunOptions runOptions{
            .sources = {
                .paths = featureFiles,
                .tagExpression = tagExpression,
            },
            .runtime = {
                .retry = 1,
            },
        };

        auto& listeners = testing::UnitTest::GetInstance()->listeners();
        auto* defaultEventListener = listeners.Release(listeners.default_result_printer());

        api::RunCucumber(runOptions, parameterRegistry, *programContext, broadcaster);

        listeners.Append(defaultEventListener);

        // if (options.printStepsNotUsed)
        //     PrintStepsNotUsed(stepRegistry);

        std::cout << '\n'
                  << std::flush;
    }

    void Application::PrintStepsNotUsed(const StepRegistry& stepRegistry) const
    {
        auto isUnused = [](const StepRegistry::EntryView& entry)
        {
            return entry.used == 0;
        };

        auto unusedSteps = stepRegistry.List() | std::views::filter(isUnused);

        if (std::ranges::empty(unusedSteps))
            std::cout << "\nAll steps have been used.";
        else
        {
            std::cout << "\nThe following steps have not been used:";
            for (const auto& entry : unusedSteps)
                std::cout << "\n - " << std::visit(cucumber_expression::SourceVisitor{}, entry.stepRegex);
        }
    }

    int Application::GetExitCode() const
    {
        return 0;
        // if (testing::UnitTest::GetInstance()->Failed())
        //     return GetExitCode(engine::Result::failed);
        // return GetExitCode(engine::Result::passed);
    }

    // int Application::GetExitCode(engine::Result result) const
    // {
    //     return static_cast<std::underlying_type_t<engine::Result>>(result) - static_cast<std::underlying_type_t<engine::Result>>(engine::Result::passed);
    // }
}
