#include "cucumber_cpp/library/Application.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/Errors.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/Matcher.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FeatureFactory.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/HookExecutor.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/TestExecution.hpp"
#include "cucumber_cpp/library/engine/TestRunner.hpp"
#include "cucumber_cpp/library/report/JunitReport.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include "cucumber_cpp/library/report/StdOutReport.hpp"
#include <CLI/Error.hpp>
#include <CLI/Option.hpp>
#include <CLI/Validators.hpp>
#include <CLI/impl/App_inl.hpp>
#include <CLI/impl/Option_inl.hpp>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <optional>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
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

    ReportHandlerValidator::ReportHandlerValidator(const report::Reporters& reporters)
        : CLI::Validator("ReportHandler", [&reporters, cachedAvailableReporters = std::optional<std::vector<std::string>>{}](const std::string& str) mutable
              {
                  if (!cachedAvailableReporters)
                      cachedAvailableReporters = reporters.AvailableReporters();

                  if (std::ranges::find(*cachedAvailableReporters, str) == cachedAvailableReporters->end())
                      return std::string{ "'" + str + "' is not a reporter" };
                  else
                      return std::string{};
              })
    {}

    Application::Application(std::shared_ptr<ContextStorageFactory> contextStorageFactory, bool removeDefaultGoogleTestListener)
        : contextManager{ std::move(contextStorageFactory) }
        , reporters{ contextManager }
        , reportHandlerValidator{ reporters }
        , removeDefaultGoogleTestListener{ removeDefaultGoogleTestListener }

    {
        gherkin.include_source(false);
        gherkin.include_ast(true);
        gherkin.include_pickles(true);

        cli.require_subcommand(1);

        runCommand = cli.add_subcommand("run")->parse_complete_callback([this]
            {
                RunFeatures();
            });

        runCommand->add_option("-t,--tag", options.tags, "Cucumber tag expression");
        runCommand->add_option("-f,--feature", options.features, "Feature file or folder with feature files")->required()->check(CLI::ExistingPath);

        runCommand->add_option("-r,--report", options.reporters, "Name of the report generator: ")->required()->group("report generation")->check(reportHandlerValidator);
        runCommand->add_option("--outputfolder", options.outputfolder, "Specifies the output folder for generated report files")->group("report generation");
        runCommand->add_option("--reportfile", options.reportfile, "Specifies the output name for generated report files")->group("report generation");
        runCommand->add_flag("--dry", options.dryrun, "Generate report without running tests");
        runCommand->add_flag("--unused", options.printStepsNotUsed, "Show step definitions that were not used")->default_val(false);

        reporters.Add("console", std::make_unique<report::StdOutReport>());
        reporters.Add("junit-xml", std::make_unique<report::JunitReport>(options.outputfolder, options.reportfile));

        ProgramContext().InsertRef(options);
    }

    int Application::Run(int argc, const char* const* argv)
    {
        try
        {
            const auto reportDescription = runCommand->get_option("--report")->get_description();
            const auto joinedReporters = reportDescription + Join(reporters.AvailableReporters(), ", ");

            runCommand->get_option("--report")->description(joinedReporters);
            cli.parse(argc, argv);
        }
        catch (const CLI::ParseError& e)
        {
            return cli.exit(e);
        }
        catch (const InternalError& error)
        {
            std::cout << "Internal error:\n\n";
            std::cout << error.what() << std::endl;
            return GetExitCode(engine::Result::failed);
        }
        catch (const UnsupportedAsteriskError& error)
        {
            std::cout << "UnsupportedAsteriskError error:\n\n";
            std::cout << error.what() << std::endl;
            return GetExitCode(engine::Result::failed);
        }
        catch (const cucumber_expression::Error& error)
        {
            std::cout << "Cucumber Expression error:\n\n";
            std::cout << error.what() << std::endl;
            return GetExitCode(engine::Result::failed);
        }
        catch (const std::exception& error)
        {
            std::cout << "Generic error:\n\n";
            std::cout << error.what() << std::endl;
            return GetExitCode(engine::Result::failed);
        }
        catch (...)
        {
            std::cout << "Unknown error";
            return GetExitCode(engine::Result::failed);
        }

        return GetExitCode();
    }

    CLI::App& Application::CliParser()
    {
        return *runCommand;
    }

    Context& Application::ProgramContext()
    {
        return contextManager.ProgramContext();
    }

    cucumber_expression::ParameterRegistration& Application::ParameterRegistration()
    {
        return parameterRegistry;
    }

    void Application::AddReportHandler(const std::string& name, std::unique_ptr<report::ReportHandlerV2>&& reporter)
    {
        reporters.Add(name, std::move(reporter));
    }

    void Application::RunFeatures()
    {
        for (const auto& selectedReporter : options.reporters)
            reporters.Use(selectedReporter);

        auto tagExpression = Join(options.tags, " ");
        engine::HookExecutorImpl hookExecution{ contextManager };

        const auto& runPolicy = (options.dryrun) ? static_cast<const engine::TestExecution::Policy&>(engine::dryRunPolicy)
                                                 : static_cast<const engine::TestExecution::Policy&>(engine::executeRunPolicy);

        std::unique_ptr<engine::TestExecutionImpl> testExecution;
        if (removeDefaultGoogleTestListener)
            testExecution = std::make_unique<engine::TestExecutionImplWithoutDefaultGoogleListener>(contextManager, reporters, hookExecution, runPolicy);
        else
            testExecution = std::make_unique<engine::TestExecutionImpl>(contextManager, reporters, hookExecution, runPolicy);

        StepRegistry stepRegistry{ parameterRegistry };
        engine::FeatureTreeFactory featureTreeFactory{ stepRegistry };

        engine::TestRunnerImpl testRunner{ featureTreeFactory, *testExecution };

        testRunner.Run(GetFeatureTree(featureTreeFactory, tagExpression));

        if (options.printStepsNotUsed)
        {
            auto isUsed = [](const StepRegistry::EntryView& entry)
            {
                return entry.used > 0;
            };
            auto unusedSteps = stepRegistry.List() | std::views::filter(std::not_fn(isUsed));
            if (std::ranges::begin(unusedSteps) == std::ranges::end(unusedSteps))
                std::cout << "\nAll steps have been used.";
            else
            {
                std::cout << "\nThe following steps have not been used:";
                for (const auto& entry : unusedSteps)
                    std::cout << "\n - " << std::visit(cucumber_expression::PatternVisitor{}, entry.stepRegex);
            }
        }

        std::cout << '\n'
                  << std::flush;
    }

    std::vector<std::unique_ptr<engine::FeatureInfo>> Application::GetFeatureTree(const engine::FeatureTreeFactory& featureTreeFactory, std::string_view tagExpression)
    {

        const auto featureFiles = GetFeatureFiles(options);
        std::vector<std::unique_ptr<engine::FeatureInfo>> vec;
        vec.reserve(featureFiles.size());

        for (const auto& featurePath : featureFiles)
            vec.push_back(featureTreeFactory.Create(featurePath, tagExpression));

        return vec;
    }

    int Application::GetExitCode() const
    {
        return GetExitCode(contextManager.ProgramContext().ExecutionStatus());
    }

    int Application::GetExitCode(engine::Result result) const
    {
        return static_cast<std::underlying_type_t<engine::Result>>(result) - static_cast<std::underlying_type_t<engine::Result>>(engine::Result::passed);
    }
}
