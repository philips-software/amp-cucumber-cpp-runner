#include "cucumber-cpp/Application.hpp"
#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/CucumberRunner.hpp"
#include "cucumber-cpp/report/JunitReport.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include "cucumber-cpp/report/StdOutReport.hpp"
#include "cucumber/gherkin/file.hpp"
#include "cucumber/gherkin/parse_error.hpp"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
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
    {
        gherkin.include_source(false);
        gherkin.include_ast(true);
        gherkin.include_pickles(true);

        cli.add_option("-t,--tag", options.tags, "Cucumber tag expression")->expected(0, -1);
        cli.add_option("-f,--feature", options.features, "Feature file or folder with feature files")->required();
        cli.add_option("-r,--report", options.reporters, "Name of the report generator: ")->required();

        reporters.Add("console", std::make_unique<report::StdOutReportV2>());
        reporters.Add("junit-xml", std::make_unique<report::JunitReportV2>());
    }

    int Application::Run(int argc, const char* const* argv)
    {
        try
        {
            const auto reportDescription = cli.get_option("--report")->get_description();
            const auto availableReporters = reporters.AvailableReporters();

            const auto joinedReporters = std::accumulate(availableReporters.begin() + 1, availableReporters.end(), reportDescription + availableReporters.front(), [](const std::string& lhs, const std::string& rhs)
                {
                    return lhs + ", " + rhs;
                });

            cli.get_option("--report")->description(joinedReporters);
            cli.parse(argc, argv);

            for (const auto& selectedReporter : options.reporters)
                reporters.Use(selectedReporter);

            RunFeatures();
        }
        catch (const CLI::ParseError& e)
        {
            return cli.exit(e);
        }

        return GetExitCode();
    }

    CLI::App& Application::CliParser()
    {
        return cli;
    }

    Context& Application::ProgramContext()
    {
        return programContext;
    }

    void Application::AddReportHandler(const std::string& name, std::unique_ptr<report::ReportHandler>&& reporter)
    {
        reporters.Add(name, std::move(reporter));
    }

    void Application::RunFeatures()
    {
        auto tagExpression = options.tags.empty() ? std::string{} : std::accumulate(std::next(options.tags.begin()), options.tags.end(), std::string(options.tags.front()), JoinStringWithSpace);

        CucumberRunnerV2 cucumberRunner{ programContext, std::move(tagExpression), reporters };

        for (const auto& featurePath : GetFeatureFiles())
            resultStatus = RunFeature(cucumberRunner, featurePath);

        if (static_cast<ResultStatus::Result>(resultStatus) == ResultStatus::Result::undefined)
            std::cout << "\nError: no features have been executed";
    }

    [[nodiscard]] report::ReportHandler::Result Application::RunFeature(CucumberRunnerV2& cucumberRunner, const std::filesystem::path& path)
    {
        std::unique_ptr<FeatureRunnerV2> featureRunner;

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
