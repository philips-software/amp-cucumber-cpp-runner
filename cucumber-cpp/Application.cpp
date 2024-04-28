#include "cucumber-cpp/Application.hpp"
#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/CucumberRunner.hpp"
#include "cucumber-cpp/engine/FeatureFactory.hpp"
#include "cucumber-cpp/report/JunitReport.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include "cucumber-cpp/report/StdOutReport.hpp"
#include "cucumber/gherkin/file.hpp"
#include "cucumber/gherkin/parse_error.hpp"
#include <CLI/Validators.hpp>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
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
            return { subrange.data(), subrange.data() + subrange.size() };
        }

        template<class Trange, class Delim>
        std::string Join(const Trange& range, const Delim& delim)
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
        : programContext{ std::move(contextStorageFactory) }
        , reportHandlerValidator{ reporters }
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

        reporters.Add("console", std::make_unique<report::StdOutReport>());
        reporters.Add("junit-xml", std::make_unique<report::JunitReport>(options.outputfolder, options.reportfile));

        programContext.InsertRef(options);
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
        return programContext;
    }

    const Application::Options& Application::CliOptions() const
    {
        return options;
    }

    void Application::AddReportHandler(const std::string& name, std::unique_ptr<report::ReportHandler>&& reporter)
    {
        reporters.Add(name, std::move(reporter));
    }

    void Application::RunFeatures()
    {
        for (const auto& selectedReporter : options.reporters)
            reporters.Use(selectedReporter);

        CucumberRunner cucumberRunner{ programContext, Join(options.tags, " "), reporters };

        for (const auto& featurePath : GetFeatureFiles())
            resultStatus = RunFeature(cucumberRunner, featurePath);

        if (static_cast<ResultStatus::Result>(resultStatus) == ResultStatus::Result::undefined)
            std::cout << "\nError: no features have been executed";
    }

    [[nodiscard]] report::ReportHandler::Result Application::RunFeature(CucumberRunner& cucumberRunner, const std::filesystem::path& path)
    {
        static engine::FeatureTreeFactory featureTreeFactory{};

        featureTreeFactory.Create(path);

        std::unique_ptr<FeatureRunner> featureRunner;

        cucumber::gherkin::app::callbacks callbacks{
            .ast = [&featureRunner, &cucumberRunner](const cucumber::gherkin::app::parser_result& ast)
            {
                featureRunner = cucumberRunner.StartFeature(ast);
            },
            .pickle = [&featureRunner](const cucumber::messages::pickle& pickle)
            {
                featureRunner->StartScenario(pickle);
            },
            .error = [](const cucumber::gherkin::parse_error& /* _ */)
            {
                /* not handled yet */
            }
        };

        gherkin.parse(cucumber::gherkin::file{ path.string() }, callbacks);

        return featureRunner->Result();
    }

    int Application::GetExitCode() const
    {
        if (resultStatus.IsSuccess())
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
