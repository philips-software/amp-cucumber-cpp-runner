#include "cucumber_cpp/library/Application.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FeatureFactory.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
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
#include <utility>
#include <vector>

namespace cucumber_cpp
{
    namespace
    {
        std::string_view const_char_to_sv(const char* value)
        {
            return { value };
        }

        std::string_view subrange_to_sv(const auto& subrange)
        {
            return { std::data(subrange), std::data(subrange) + std::size(subrange) };
        }

        template<class Range, class Delim>
        std::string Join(const Range& range, const Delim& delim)
        {
            if (range.empty())
                return "";

            return std::accumulate(std::next(range.begin()), range.end(), range.front(), [&delim](const auto& lhs, const auto& rhs)
                {
                    return lhs + delim + rhs;
                });
        }

        std::string JoinStringWithSpace(const std::string& a, const std::string& b)
        {
            return a + ' ' + b;
        }

        std::filesystem::path to_fs_path(const std::string_view& sv)
        {
            return { sv };
        }

        bool is_feature_file(const std::filesystem::directory_entry& entry)
        {
            return std::filesystem::is_regular_file(entry) && entry.path().has_extension() && entry.path().extension() == ".feature";
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
    {
    }

    ResultStatus& ResultStatus::operator=(Result result)
    {
        if ((resultStatus == Result::undefined || resultStatus == Result::success) && result != Result::undefined)
            resultStatus = result;

        return *this;
    }

    ResultStatus::operator Result() const
    {
        return resultStatus;
    }

    bool ResultStatus::IsSuccess() const
    {
        return resultStatus == Result::success;
    }

    Application::Application(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
        : reportHandlerValidator{ reporters }
        , contextManager{ std::move(contextStorageFactory) }
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

        return GetExitCode();
    }

    CLI::App& Application::CliParser()
    {
        return *runCommand;
    }

    Context& Application::ProgramContext()
    {
        return contextManager.CurrentContext();
    }

    const Application::Options& Application::CliOptions() const
    {
        return options;
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

        if (options.dryrun)
            engine::TestRunner::Run(contextManager, GetFeatureTree(tagExpression), reporters, engine::dryRun);
        else
            engine::TestRunner::Run(contextManager, GetFeatureTree(tagExpression), reporters, engine::runTest);

        std::cout << '\n'
                  << std::flush;
    }

    std::vector<std::unique_ptr<engine::FeatureInfo>> Application::GetFeatureTree(std::string_view tagExpression)
    {
        auto featureFiles = GetFeatureFiles();
        std::vector<std::unique_ptr<engine::FeatureInfo>> vec;
        vec.reserve(featureFiles.size());

        for (const auto& featurePath : featureFiles)
            vec.push_back(featureTreeFactory.Create(featurePath, tagExpression));

        vec.shrink_to_fit();

        return vec;
    }

    int Application::GetExitCode() const
    {
        if (contextManager.CurrentContext().ExecutionStatus() == engine::Result::passed)
            return 0;
        else
            return 1;
    }

    std::vector<std::filesystem::path> Application::GetFeatureFiles() const
    {
        std::vector<std::filesystem::path> files;

        for (const auto feature : options.features | std::views::transform(to_fs_path))
            if (std::filesystem::is_directory(feature))
                for (const auto& entry : std::filesystem::directory_iterator{ feature } | std::views::filter(is_feature_file))
                    files.emplace_back(entry.path());
            else
                files.emplace_back(feature);

        return files;
    }
}
