#include "cucumber_cpp/library/Application.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/Errors.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/api/RunCucumber.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/tag_expression/Parser.hpp"
#include <CLI/App.hpp>
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
#include <map>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <variant>

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

        std::set<std::filesystem::path> GetFeatureFiles(Application::Options& options)
        {
            std::set<std::filesystem::path> files;

            for (const auto feature : options.paths | std::views::transform(ToFileSystemPath))
                if (std::filesystem::is_directory(feature))
                    for (const auto& entry : std::filesystem::directory_iterator{ feature } | std::views::filter(IsFeatureFile))
                        files.emplace(entry.path());
                else
                    files.emplace(feature);

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
    }

    int Application::Run(int argc, const char* const* argv)
    {
        try
        {
            const std::map<std::string, support::RunOptions::Ordering> orderingMap{
                { "defined", support::RunOptions::Ordering::defined },
                { "reverse", support::RunOptions::Ordering::reverse },
            };

            runCommand->add_flag("-d,--dry-run", options.dryRun, "Perform a dry run without executing steps");
            runCommand->add_flag("--fail-fast", options.failFast, "Stop execution on first failure");
            runCommand->add_option("--format", options.format, "specify the output format, optionally supply PATH to redirect formatter output.  Available formats:\n");
            runCommand->add_option("--format-options", options.formatOptions, "provide options for formatters");
            runCommand->add_option("--language", options.language, "Default langauge for feature files, eg 'en'")->default_str(options.language);
            runCommand->add_option("--order", options.ordering, "Run scenarios in specificed order")->transform(CLI::CheckedTransformer(orderingMap, CLI::ignore_case));
            auto* retryOpt = runCommand->add_option("--retry", options.retry, "Number of times to retry failed scenarios")->default_val(options.retry);
            runCommand->add_option("--retry-tag-filter", options.retryTagFilter, "Only retry scenarios matching this tag expression")->needs(retryOpt);
            runCommand->add_flag("--strict,!--no-strict", options.strict, "Fail if there are pending steps")->default_val(options.strict);

            CLI::deprecate_option(runCommand->add_option("--tag", options.tags, "Cucumber tag expression"), "-t,--tags");
            runCommand->add_option("-t,--tags", options.tags, "Cucumber tag expression");

            CLI::deprecate_option(runCommand->add_option("-f,--feature", options.paths, "Paths to where your feature files are")->check(CLI::ExistingPath), "paths");
            runCommand->add_option("paths", options.paths, "Paths to where your feature files are")->required()->check(CLI::ExistingPath);

            // runCommand->add_option("-r,--report", options.reporters, "Name of the report generator: ")->required()->group("report generation"); //->check(reportHandlerValidator);
            // runCommand->add_option("--outputfolder", options.outputfolder, "Specifies the output folder for generated report files")->group("report generation");
            // runCommand->add_option("--reportfile", options.reportfile, "Specifies the output name for generated report files")->group("report generation");
            // runCommand->add_flag("--unused", options.printStepsNotUsed, "Show step definitions that were not used");

            // reporters.Add("console", std::make_unique<report::StdOutReport>());
            // reporters.Add("junit-xml", std::make_unique<report::JunitReport>(options.outputfolder, options.reportfile));

            ProgramContext().InsertRef(options);

            // const auto reportDescription = runCommand->get_option("--report")->get_description();
            // const auto joinedReporters = reportDescription; // + Join(reporters.AvailableReporters(), ", ");

            // runCommand->get_option("--report")->description(joinedReporters);
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

    cucumber_expression::ParameterRegistry& Application::ParameterRegistration()
    {
        return parameterRegistry;
    }

    // void Application::AddReportHandler(const std::string& name, std::unique_ptr<report::ReportHandlerV2>&& reporter)
    // {
    //     reporters.Add(name, std::move(reporter));
    // }

    void Application::RunFeatures()
    {
        const auto runOptions = support::RunOptions{
            .sources = {
                .paths = GetFeatureFiles(options),
                .tagExpression = tag_expression::Parse(Join(options.tags, " ")),
                .ordering = options.ordering,
            },
            .runtime = {
                .dryRun = options.dryRun,
                .failFast = options.failFast,
                .retry = options.retry,
                .strict = options.strict,
                .retryTagExpression = tag_expression::Parse(Join(options.retryTagFilter, " ")),
            },
        };

        auto& listeners = testing::UnitTest::GetInstance()->listeners();
        auto* defaultEventListener = listeners.Release(listeners.default_result_printer());

        runPassed = api::RunCucumber(runOptions, parameterRegistry, *programContext, broadcaster, formatters, options.format, options.formatOptions);

        listeners.Append(defaultEventListener);

        // if (options.printStepsNotUsed)
        //     PrintStepsNotUsed(stepRegistry);
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
        return runPassed ? 0 : 1;
    }
}
